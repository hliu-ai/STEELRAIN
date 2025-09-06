import torch as T
import torch.nn as nn
import torch.nn.functional as F
from torch.distributions import Normal, Categorical
import torch.optim as optim

class ActorNetwork(nn.Module):
    def __init__(self, lr, input_dims, max_action, fc1_dims=256, fc2_dims=256): #adjust max action if want scaling
        super(ActorNetwork, self).__init__()
        self.fc1 = nn.Linear(*input_dims, fc1_dims)
        self.fc2 = nn.Linear(fc1_dims, fc2_dims)

        # Continuous head: outputs mean and sigma for two parameters.
        self.mu = nn.Linear(fc2_dims, 2)
        self.sigma = nn.Linear(fc2_dims, 2)
        # Discrete head: outputs logits for two discrete actions.
        self.discrete = nn.Linear(fc2_dims, 2)

        self.max_action = T.tensor(max_action)

    def encode(self, state):
        x = F.relu(self.fc1(state))
        x = F.relu(self.fc2(x))
        return x

    def forward(self, state, dist=False):
        x = self.encode(state)
        mu = T.tanh(self.mu(x)) * self.max_action.to(state.device)
        sigma = F.softplus(self.sigma(x))
        continuous_dist = Normal(mu, sigma)
        
        discrete_logits = self.discrete(x)
        discrete_dist = Categorical(logits=discrete_logits)

        if dist:
            return continuous_dist, discrete_dist
        else:
            return continuous_dist.sample(), discrete_dist.sample()

    def sample(self, state):
        continuous_dist, discrete_dist = self.forward(state, dist=True)

        #print("continuous_dist.mu shape:", continuous_dist.mean.shape)  # all [1, 2]
        #print("continuous_dist.sigma shape:", continuous_dist.stddev.shape)  # if available
        #print("discrete_dist.logits shape:", discrete_dist.logits.shape)
    

        continuous_action = continuous_dist.sample()
        discrete_action = discrete_dist.sample()

        #print("Sampled continuous_action shape:", continuous_action.shape)  # [1, 2]
        #print("Sampled discrete_action shape:", discrete_action.shape)      # [1]

        log_prob_c = continuous_dist.log_prob(continuous_action).sum(dim=-1)
        log_prob_d = discrete_dist.log_prob(discrete_action)

        #print("Computed log_prob_c shape:", log_prob_c.shape)  # [1]
        #print("Computed log_prob_d shape:", log_prob_d.shape)  # [1]
        # Return discrete action, continuous action, and their log probabilities.
        return discrete_action, continuous_action, log_prob_d, log_prob_c

    def get_continuous_distribution(self, state):
        x = self.encode(state)
        mu = T.tanh(self.mu(x)) * self.max_action.to(state.device)
        #print("max_action value:", self.max_action, "max_Action shape: ", self.max_action.shape)
        sigma = F.softplus(self.sigma(x))
        #print("mu shape:", mu.shape)
        #print("sigma shape: ", sigma.shape)
        return Normal(mu, sigma)

    def get_discrete_distribution(self, state):
        x = self.encode(state)
        discrete_logits = self.discrete(x)
        return Categorical(logits=discrete_logits)

    def discrete_parameters(self):
        return list(self.fc1.parameters()) + list(self.fc2.parameters()) + list(self.discrete.parameters())

    def continuous_parameters(self):
        return list(self.fc1.parameters()) + list(self.fc2.parameters()) + list(self.mu.parameters()) + list(self.sigma.parameters())

    def save_checkpoint(self, path):
        T.save(self.state_dict(), f"{path}/actor.pth")

    def load_checkpoint(self, path):
        self.load_state_dict(T.load(f"{path}/actor.pth"))

class CriticNetwork(nn.Module):
    def __init__(self, lr, input_dims, fc1_dims=256, fc2_dims=256):
        super(CriticNetwork, self).__init__()
        self.fc1 = nn.Linear(*input_dims, fc1_dims)
        self.fc2 = nn.Linear(fc1_dims, fc2_dims)
        self.v = nn.Linear(fc2_dims, 1)
        self.optimizer = optim.Adam(self.parameters(), lr=lr)

    def forward(self, state):
        x = F.relu(self.fc1(state))
        x = F.relu(self.fc2(x))
        return self.v(x)

    def save_checkpoint(self, path):
        T.save(self.state_dict(), f"{path}/critic.pth")

    def load_checkpoint(self, path):
        self.load_state_dict(T.load(f"{path}/critic.pth"))
