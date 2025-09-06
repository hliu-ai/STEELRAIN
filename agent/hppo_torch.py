import torch as T
import torch.optim as optim
import numpy as np
from networks import ActorNetwork, CriticNetwork
from torch.distributions import TransformedDistribution
from buffer import PPOBuffer
from torch.nn.utils import clip_grad_norm_

# Entropy bonus coefficients (to encourage stable exploration)
BETA_D = 0.01  # discrete entropy coefficient
BETA_C = 0.005  # continuous entropy coefficient

class PPOAgent:
    def __init__(
        self, input_dims, env, lr_actor=1e-4, lr_critic=1e-3,
        gamma=0.99, gae_lambda=0.95, policy_clip=0.2,
        batch_size=128, n_epochs=10
    ):
        self.gamma = gamma
        self.gae_lambda = gae_lambda
        self.policy_clip = policy_clip
        self.batch_size = batch_size
        self.n_epochs = n_epochs
        self.device = T.device('cuda' if T.cuda.is_available() else 'cpu')

        # Actor and critic networks
        self.actor = ActorNetwork(
            lr_actor, input_dims,
            env.action_space['continuous'].high
        ).to(self.device)
        self.critic = CriticNetwork(lr_critic, input_dims).to(self.device)

        # Optimizer for actor: separate parameter groups
        shared_params = list(self.actor.fc1.parameters()) + list(self.actor.fc2.parameters())
        discrete_params = list(self.actor.discrete.parameters())
        continuous_params = list(self.actor.mu.parameters()) + list(self.actor.sigma.parameters())
        self.actor_optimizer = optim.Adam([
            {'params': shared_params,   'lr': lr_actor},
            {'params': discrete_params, 'lr': lr_actor},
            {'params': continuous_params,'lr': lr_actor}
        ])

        # Critic optimizer
        self.critic_optimizer = optim.Adam(self.critic.parameters(), lr=lr_critic)

        # Buffer for PPO
        self.buffer = PPOBuffer(batch_size, input_dims, gamma, gae_lambda, self.device)

        # Metrics
        metrics = ['discrete_loss','continuous_loss','actor_loss','critic_loss','total_loss',
                   'discrete_log_prob','continuous_log_prob','actor_grad_norm','critic_grad_norm',
                   'kl_d','kl_c','discrete_entropy','continuous_entropy','value_error']
        for m in metrics:
            setattr(self, f"last_{m}", 0.0)
        # Track the mean sigma of the last update
        self.last_sigma = 0.0


    def choose_action(self, observation):
        state = T.tensor(np.array(observation), dtype=T.float32)
        state = state.unsqueeze(0).to(self.device)
        discrete_action, continuous_action, log_prob_d, log_prob_c = self.actor.sample(state)
        value = self.critic(state)

        action = {
            'discrete': int(discrete_action.cpu().detach().numpy()[0]),
            'continuous': continuous_action.cpu().detach().numpy()[0]
        }

        return (
            action,
            log_prob_d.cpu().detach().numpy()[0],   # detach before numpy
            log_prob_c.cpu().detach().numpy()[0],   # detach before numpy
            value.cpu().detach().item()             # detach for consistency
        )


    def learn(self):
        states, actions, old_log_probs_d, old_log_probs_c, returns, advantages = \
            self.buffer.generate_batches()

        # Accumulators
        total_actor_loss = total_critic_loss = 0.0
        total_batches = total_discrete_log_prob = total_continuous_log_prob = 0.0
        total_actor_grad_norm = total_critic_grad_norm = 0.0
        total_kl_d = total_kl_c = 0.0
        total_discrete_entropy = total_continuous_entropy = 0.0
        total_value_error = 0.0

        for _ in range(self.n_epochs):
            for start in range(0, len(states), self.batch_size): #steps by batch_size = 128 each time, slices em up. 
                end = start + self.batch_size
                s_batch = states[start:end]
                a_batch = actions[start:end]
                old_log_d = old_log_probs_d[start:end]
                old_log_c = old_log_probs_c[start:end]
                ret_batch = returns[start:end]
                adv_batch = advantages[start:end]

                # Unpack actions
                discrete_actions = a_batch[:, 0].long()
                continuous_actions = a_batch[:, 1:]

                # Discrete PPO loss
                d_dist = self.actor.get_discrete_distribution(s_batch)
                new_log_d = d_dist.log_prob(discrete_actions)
                ratio_d = T.exp(new_log_d - old_log_d)
                surr1_d = ratio_d * adv_batch
                surr2_d = T.clamp(ratio_d, 1 - self.policy_clip, 1 + self.policy_clip) * adv_batch
                disc_loss = -T.min(surr1_d, surr2_d).mean()

                # Continuous PPO loss
                c_dist = self.actor.get_continuous_distribution(s_batch)
                new_log_c = c_dist.log_prob(continuous_actions).sum(dim=-1)
                ratio_c = T.exp(new_log_c - old_log_c)
                surr1_c = ratio_c * adv_batch
                surr2_c = T.clamp(ratio_c, 1 - self.policy_clip, 1 + self.policy_clip) * adv_batch
                cont_loss = -T.min(surr1_c, surr2_c).mean()

                # Entropies and KL
                disc_entropy = d_dist.entropy().mean()
                try:
                    cont_entropy = c_dist.entropy().mean()
                except NotImplementedError:
                    base = c_dist
                    while isinstance(base, TransformedDistribution):
                        base = base.base_dist
                    cont_entropy = base.entropy().sum(dim=-1).mean()
                kl_d = T.mean(ratio_d - 1 - T.log(ratio_d))
                kl_c = T.mean(ratio_c - 1 - T.log(ratio_c))

                # Combine actor losses with entropy bonuses
                actor_loss = disc_loss + cont_loss \
                             - BETA_D * disc_entropy \
                             - BETA_C * cont_entropy  # ENTROPY BONUS

                # Update actor
                self.actor_optimizer.zero_grad()
                actor_loss.backward()
                #gradient clipping
                clip_grad_norm_(self.actor.parameters(), max_norm=0.5)  # <-- cap at 0.5

                actor_grad_norm = T.linalg.norm(
                    T.stack([p.grad.norm() for p in self.actor.parameters() if p.grad is not None])
                ).item()
                self.actor_optimizer.step()

                # Critic update
                values = self.critic(s_batch).squeeze()
                critic_loss = (ret_batch - values).pow(2).mean()
                value_error = T.mean(T.abs(ret_batch - values)).item()
                self.critic_optimizer.zero_grad()
                critic_loss.backward()
                critic_grad_norm = T.linalg.norm(
                    T.stack([p.grad.norm() for p in self.critic.parameters() if p.grad is not None])
                ).item()
                self.critic_optimizer.step()

                # Accumulate metrics
                total_actor_loss += actor_loss.item()
                total_critic_loss += critic_loss.item()
                total_discrete_log_prob += new_log_d.mean().item()
                total_continuous_log_prob += new_log_c.mean().item()
                total_actor_grad_norm += actor_grad_norm
                total_critic_grad_norm += critic_grad_norm
                total_kl_d += kl_d.item()
                total_kl_c += kl_c.item()
                total_discrete_entropy += disc_entropy.item()
                total_continuous_entropy += cont_entropy.item()
                total_value_error += value_error
                total_batches += 1

        # Write back averaged metrics
        avg = lambda x: x / total_batches if total_batches > 0 else 0
        self.last_discrete_loss = disc_loss.item()
        self.last_continuous_loss = cont_loss.item()
        self.last_actor_loss = avg(total_actor_loss)
        self.last_critic_loss = avg(total_critic_loss)
        self.last_total_loss = self.last_actor_loss + 0.5 * self.last_critic_loss
        self.last_discrete_log_prob = avg(total_discrete_log_prob)
        self.last_continuous_log_prob = avg(total_continuous_log_prob)
        self.last_actor_grad_norm = avg(total_actor_grad_norm)
        self.last_critic_grad_norm = avg(total_critic_grad_norm)
        self.last_kl_d = avg(total_kl_d)
        self.last_kl_c = avg(total_kl_c)
        self.last_discrete_entropy = avg(total_discrete_entropy)
        self.last_continuous_entropy = avg(total_continuous_entropy)
        self.last_value_error = avg(total_value_error)
        # ---- track sigma from last batch ----
        base = c_dist
        while hasattr(base, 'base_dist'):
            base = base.base_dist
        # base.scale is shape (batch,2); take its mean over batch and dims
        self.last_sigma = base.scale.mean().item()

        self.buffer.clear()

    def save_models(self, path):
        self.actor.save_checkpoint(path)
        self.critic.save_checkpoint(path)

    def load_models(self, path):
        self.actor.load_checkpoint(path)
        self.critic.load_checkpoint(path)
