from hmac import new
import torch as T
import torch.optim as optim
import numpy as np
from networks import ActorNetwork, CriticNetwork
from buffer import PPOBuffer

class PPOAgent():
    def __init__(self, input_dims, env, lr_actor=1e-4, lr_critic=1e-3,
                 gamma=0.99, gae_lambda=0.95, policy_clip=0.2,
                 batch_size=64, n_epochs=10):
        self.gamma = gamma
        self.gae_lambda = gae_lambda
        self.policy_clip = policy_clip
        self.batch_size = batch_size
        self.n_epochs = n_epochs
        self.device = T.device('cuda' if T.cuda.is_available() else 'cpu')

        # Actor network with shared encoder and two heads.
        self.actor = ActorNetwork(lr_actor, input_dims, env.action_space['continuous'].high).to(self.device)
        self.critic = CriticNetwork(lr_critic, input_dims).to(self.device)

        # Separate shared encoder and head-specific parameters.
        shared_params = list(self.actor.fc1.parameters()) + list(self.actor.fc2.parameters())
        discrete_params = list(self.actor.discrete.parameters())
        continuous_params = list(self.actor.mu.parameters()) + list(self.actor.sigma.parameters())
        
        # Create a single optimizer with three parameter groups.
        self.actor_optimizer = optim.Adam([
            {'params': shared_params, 'lr': lr_actor},
            {'params': discrete_params, 'lr': lr_actor},
            {'params': continuous_params, 'lr': lr_actor}
        ])

        # Critic network uses its own optimizer (stored in self.critic.optimizer).
        self.buffer = PPOBuffer(batch_size, input_dims, gamma, gae_lambda, self.device)

        # Metrics for logging.
        self.last_discrete_loss = 0.0
        self.last_continuous_loss = 0.0
        self.last_actor_loss = 0.0
        self.last_critic_loss = 0.0
        self.last_total_loss = 0.0
        self.last_discrete_log_prob = 0.0
        self.last_continuous_log_prob = 0.0

        # New metrics:
        self.last_actor_grad_norm = 0.0
        self.last_critic_grad_norm = 0.0
        self.last_kl_d = 0.0
        self.last_kl_c = 0.0
        self.last_discrete_entropy = 0.0
        self.last_continuous_entropy = 0.0
        self.last_value_error = 0.0

    def choose_action(self, observation):
        state = T.tensor(np.array(observation), dtype=T.float32).unsqueeze(0).to(self.device)
        #print("State shape after unsqueeze:", state.shape)  # Expected: [1, 405]
        # sample() returns: discrete_action, continuous_action, log_prob_d, log_prob_c.
        discrete_action, continuous_action, log_prob_d, log_prob_c = self.actor.sample(state)


        #print("Discrete action shape:", discrete_action.shape)        #  [1]
        #print("Continuous action shape:", continuous_action.shape)      #  [1, 2]
        #print("Log_prob_d shape:", log_prob_d.shape)                    # [1]
        #print("Log_prob_c shape:", log_prob_c.shape)                    #  [1]
    
        value = self.critic(state)
        #print("Value before detach (shape):", value.shape)              # [1, 1]
        
        action = {
            'discrete': discrete_action.cpu().detach().numpy()[0],
            'continuous': continuous_action.cpu().detach().numpy()[0]
        }

        #print("Action (discrete) type & value:", type(action['discrete']), action['discrete']) #integer
        #print("Action (continuous) shape & value:", np.array(action['continuous']).shape, action['continuous']) #array (2,)


        return action, log_prob_d.cpu().detach().numpy()[0], log_prob_c.cpu().detach().numpy()[0], value.cpu().detach().item()

    def learn(self):
        # Retrieve batches: states, actions, old_log_probs_d, old_log_probs_c, returns, advantages. All tensors.
        states, actions, old_log_probs_d, old_log_probs_c, returns, advantages = self.buffer.generate_batches()
        '''
        print("States device:", states.device)
        print("Actions device:", actions.device)
        print("Log_probs_d device:", old_log_probs_d.device)
        print("Log_probs_c device:", old_log_probs_c.device)
        print("Returns device:", returns.device)
        print("Advantages device:", advantages.device)
        '''
        #TensorBoard Initializations
        total_actor_loss = 0.0
        total_critic_loss = 0.0
        total_batches = 0
        total_discrete_log_prob = 0.0
        total_continuous_log_prob = 0.0
        total_actor_grad_norm = 0.0
        total_critic_grad_norm = 0.0
        total_kl_d = 0.0
        total_kl_c = 0.0
        total_discrete_entropy = 0.0
        total_continuous_entropy = 0.0
        total_value_error = 0.0

        print(len(states))
        for _ in range(self.n_epochs):
            for batch_start in range(0, len(states), self.batch_size): #range parameters are (start, stop, step). so with current parameters, - 0, 2048, 64. batch_start is the index marking where we put the knife to slice up the 2048 pie into slices of 64
                batch_end = batch_start + self.batch_size
                state_batch = states[batch_start:batch_end]
                action_batch = actions[batch_start:batch_end]
                old_log_prob_d_batch = old_log_probs_d[batch_start:batch_end]
                old_log_prob_c_batch = old_log_probs_c[batch_start:batch_end]
                return_batch = returns[batch_start:batch_end]
                advantage_batch = advantages[batch_start:batch_end]

                '''
                print("Mini-batch shapes:")
                print("  state_batch:", state_batch.shape)
                print("  action_batch:", action_batch.shape)
                print("  old_log_prob_d_batch:", old_log_prob_d_batch.shape)
                print("  old_log_prob_c_batch:", old_log_prob_c_batch.shape)
                print("  return_batch:", return_batch.shape)
                print("  advantage_batch:", advantage_batch.shape)
                '''

                # action_batch: column 0 = discrete action; columns 1+ = continuous parameters.
                discrete_actions = action_batch[:, 0].long()
                continuous_actions = action_batch[:, 1:]

                #print("Discrete actions shape:", discrete_actions.shape)
                #print("Continuous actions shape:", continuous_actions.shape)


                # --- Discrete Policy Loss ---
                discrete_dist = self.actor.get_discrete_distribution(state_batch)
                #print("Discrete distribution logits:", discrete_dist.logits)
                #print("Discrete distribution probabilities:", discrete_dist.probs)
                
                new_log_prob_d = discrete_dist.log_prob(discrete_actions)
                #print("new log prob d shape: ", new_log_prob_d.shape) - batchsize

                ratio_d = T.exp(new_log_prob_d - old_log_prob_d_batch)
                #print("ratio d shape: ", ratio_d.shape)

                surrogate1_d = ratio_d * advantage_batch
                #print("surrogate 1 d shape: ", surrogate1_d.shape)
                surrogate2_d = T.clamp(ratio_d, 1 - self.policy_clip, 1 + self.policy_clip) * advantage_batch #clamps ratio between 0.8 and 1.2, providing a guardrail to limit how much the policy is allowed to change in a single update. stabilizes trainingf
                #print("surrogate 2 d shape: ", surrogate2_d.shape)
                discrete_loss = -T.min(surrogate1_d, surrogate2_d).mean()



                # --- Continuous Policy Loss ---
                continuous_dist = self.actor.get_continuous_distribution(state_batch)
                new_log_prob_c = continuous_dist.log_prob(continuous_actions).sum(dim=-1)
                #print("value: ", new_log_prob_c, "shape:" , new_log_prob_c.shape)
                ratio_c = T.exp(new_log_prob_c - old_log_prob_c_batch)
                surrogate1_c = ratio_c * advantage_batch
                surrogate2_c = T.clamp(ratio_c, 1 - self.policy_clip, 1 + self.policy_clip) * advantage_batch
                continuous_loss = -T.min(surrogate1_c, surrogate2_c).mean()

                # Combine actor losses.
                actor_loss = discrete_loss + continuous_loss

                # Additional metrics.
                discrete_entropy = discrete_dist.entropy().mean()
                continuous_entropy = continuous_dist.entropy().mean()
                kl_d = T.mean(ratio_d - 1 - T.log(ratio_d))
                kl_c = T.mean(ratio_c - 1 - T.log(ratio_c))

                # --- Update Actor (combined) ---
                self.actor_optimizer.zero_grad()
                actor_loss.backward()
                # Compute actor gradient norm.
                actor_grad_norm = 0.0
                for param in self.actor.parameters():
                    if param.grad is not None:
                        actor_grad_norm += param.grad.data.norm(2).item() ** 2
                actor_grad_norm = actor_grad_norm ** 0.5
                self.actor_optimizer.step()

                # --- Critic Loss ---
                values = self.critic(state_batch).squeeze()
                #print("values: ", values, " values shape: ", values.shape)

                
                critic_loss = (return_batch - values).pow(2).mean()
                # Value prediction error (mean absolute error) for tensorboard
                value_error = T.mean(T.abs(return_batch - values)).item()

                # --- Update Critic ---
                self.critic.optimizer.zero_grad()
                critic_loss.backward()
                critic_grad_norm = 0.0
                for param in self.critic.parameters():
                    if param.grad is not None:
                        critic_grad_norm += param.grad.data.norm(2).item() ** 2
                critic_grad_norm = critic_grad_norm ** 0.5
                self.critic.optimizer.step()

                total_actor_loss += actor_loss.item()
                total_critic_loss += critic_loss.item()
                total_discrete_log_prob += new_log_prob_d.mean().item()
                total_continuous_log_prob += new_log_prob_c.mean().item()
                total_actor_grad_norm += actor_grad_norm
                total_critic_grad_norm += critic_grad_norm
                total_kl_d += kl_d.item()
                total_kl_c += kl_c.item()
                total_discrete_entropy += discrete_entropy.item()
                total_continuous_entropy += continuous_entropy.item()
                total_value_error += value_error

                total_batches += 1

        self.last_discrete_loss = discrete_loss.item()
        self.last_continuous_loss = continuous_loss.item()
        self.last_actor_loss = total_actor_loss / total_batches if total_batches > 0 else 0
        self.last_critic_loss = total_critic_loss / total_batches if total_batches > 0 else 0
        self.last_total_loss = self.last_actor_loss + 0.5 * self.last_critic_loss
        self.last_discrete_log_prob = total_discrete_log_prob / total_batches if total_batches > 0 else 0
        self.last_continuous_log_prob = total_continuous_log_prob / total_batches if total_batches > 0 else 0

        self.last_actor_grad_norm = total_actor_grad_norm / total_batches if total_batches > 0 else 0
        self.last_critic_grad_norm = total_critic_grad_norm / total_batches if total_batches > 0 else 0
        self.last_kl_d = total_kl_d / total_batches if total_batches > 0 else 0
        self.last_kl_c = total_kl_c / total_batches if total_batches > 0 else 0
        self.last_discrete_entropy = total_discrete_entropy / total_batches if total_batches > 0 else 0
        self.last_continuous_entropy = total_continuous_entropy / total_batches if total_batches > 0 else 0
        self.last_value_error = total_value_error / total_batches if total_batches > 0 else 0

        
        #print("State batch shape:", state_batch.shape)
        #print("Discrete actions shape:", discrete_actions.shape)
        #print("New log probability (discrete) mean:", new_log_prob_d.mean().item())
        #print("Actor loss:", actor_loss.item())
        #print("Critic loss:", critic_loss.item())

        self.buffer.clear() #At the end of learning all batches (64) of the trajectory (2048) flush and continue.

    def save_models(self, path):
        self.actor.save_checkpoint(path)
        self.critic.save_checkpoint(path)

    def load_models(self, path):
        self.actor.load_checkpoint(path)
        self.critic.load_checkpoint(path)
