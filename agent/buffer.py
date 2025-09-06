import numpy as np
import torch as T

class PPOBuffer:
    def __init__(self, batch_size, input_dims, gamma, gae_lambda, device):
        self.batch_size = batch_size
        self.gamma = gamma
        self.gae_lambda = gae_lambda
        self.device = device

        self.states = []
        self.actions = []
        # Separate log probabilities for discrete and continuous actions.
        self.log_probs_d = []
        self.log_probs_c = []
        self.rewards = []
        self.values = []
        self.dones = []
        self.advantages = None
        self.returns = None

    def store(self, state, action, log_prob_d, log_prob_c, reward, value, done):
        self.states.append(state)
        # Clip the discrete action to ensure it lies in [0, 1]. Clipping is essentially clamping. Its just a safeguard in this case because the pytorch categorical should always give valid integers.
        discrete_action = int(np.clip(action['discrete'], 0, 1))

        action_flat = np.concatenate([np.array([discrete_action], dtype=np.float32),
                                      np.array(action['continuous'], dtype=np.float32)])
        self.actions.append(action_flat)
        self.log_probs_d.append(log_prob_d) #float
        self.log_probs_c.append(log_prob_c)
        self.rewards.append(reward)
        self.values.append(value)
        self.dones.append(done)

        '''
        print("DEBUG: Storing transition:")
        print("  - State type:", type(state), "shape:", np.array(state).shape)
        print("  - Action (discrete) before clipping:", action['discrete'], "discrete shape: ", np.array(action['discrete']))
        print("  - Action (continuous) shape:", np.array(action['continuous']).shape)
        print("  - Action flat shape:", action_flat.shape, "value:", action_flat)
        print("  - Log_prob_d type:", type(log_prob_d), "value:", log_prob_d)
        print("  - Log_prob_c type:", type(log_prob_c), "value:", log_prob_c)
        print("  - Reward:", reward)
        print("  - Value (critic):", value)
        print("  - Done flag:", done)
        '''
    

    def finish_trajectory(self, last_value):
        rewards = np.array(self.rewards, dtype=np.float32)
        values = np.array(self.values + [last_value], dtype=np.float32)
        dones = np.array(self.dones, dtype=np.float32)

        #print("Rewards shape:", rewards.shape)        # Expect: (N,)
        #print("Values shape (with last_value):", values.shape)  # Expect: (N+1,)
        #print("Dones shape:", dones.shape)              # Expect: (N,)

        advantages = np.zeros_like(rewards, dtype=np.float32)
        gae = 0

        # compute GAE in reverse
        for t in reversed(range(len(rewards))):
            delta = rewards[t] + self.gamma * values[t+1] * (1 - dones[t]) - values[t]
            gae = delta + self.gamma * self.gae_lambda * (1 - dones[t]) * gae
            advantages[t] = gae

                         # Print details for a few time steps for debugging.
            #if t % 100 == 0:  # adjust modulus for frequency of prints if needed
                #print(f"t={t}, delta={delta:.4f}, gae={gae:.4f}, advantage[{t}]={advantages[t]:.4f}")


        # compute returns from un-normalized advantages
        returns = advantages + values[:-1]

        # ----- ADVANTAGE NORMALIZATION (apparently, best practice)-----
        adv_mean = advantages.mean()
        adv_std  = advantages.std() + 1e-8
        advantages = (advantages - adv_mean) / adv_std
        # -----------------------------------

        self.advantages = advantages
        self.returns    = returns

        '''
        print("DEBUG: Finishing trajectory:")
        print("  - Rewards array shape:", np.array(self.rewards).shape)
        print("  - Values array shape (including last value):", np.array(self.values + [last_value]).shape)
        print("  - Dones array shape:", np.array(self.dones).shape)
        print("  - Computed Advantages shape:", advantages.shape)
        print("  - Computed Returns shape:", returns.shape)
        '''


    def generate_batches(self):
        # Convert lists to arrays; clip discrete action column to [0, 1] for safety.
        states = T.tensor(np.array(self.states), dtype=T.float32).to(self.device)
        
        

        actions_arr = np.array(self.actions, dtype=np.float32)
        actions_arr[:, 0] = np.clip(actions_arr[:, 0], 0, 1)

        actions = T.tensor(actions_arr, dtype=T.float32).to(self.device)
        log_probs_d = T.tensor(np.array(self.log_probs_d), dtype=T.float32).to(self.device)
       
        log_probs_c = T.tensor(np.array(self.log_probs_c), dtype=T.float32).to(self.device)
       
        returns = T.tensor(np.array(self.returns), dtype=T.float32).to(self.device)
        
        advantages = T.tensor(np.array(self.advantages), dtype=T.float32).to(self.device)

        
        print("States tensor shape:", states.shape)  # Should output: (N, 1112)
        print("Actions array shape:", actions_arr.shape)  # Should output: (N, 3)
        print("Discrete action values after clipping:", actions_arr[:, 0])
        print("Log_probs_d shape:", log_probs_d.shape)  # Should output: (N,)
        print("Log_probs_c shape:", log_probs_c.shape)  # Should output: (N,)
        print("Returns shape:", returns.shape)  # Should output: (N,)
        print("Advantages shape:", advantages.shape)  # Should output: (N,)
        

        return states, actions, log_probs_d, log_probs_c, returns, advantages

    def clear(self):
        self.states = []
        self.actions = []
        self.log_probs_d = []
        self.log_probs_c = []
        self.rewards = []
        self.values = []
        self.dones = []
        self.advantages = None
        self.returns = None
