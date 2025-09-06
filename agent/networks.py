import torch as T
import torch.nn as nn
import torch.nn.functional as F
from torch.distributions import Normal, Categorical, TransformedDistribution
from torch.distributions.transforms import TanhTransform, AffineTransform

# Hyperparameters for sigma clamping
MIN_SIGMA = 1e-2 #raised by 10x
MAX_SIGMA = 1.0

# Grid dimensions and scalar count
PERIPH_H = 27
PERIPH_W = 41
SCALAR_DIM = 5

class ActorNetwork(nn.Module):
    def __init__(self, lr, input_dims, max_action, fc1_dims=256, fc2_dims=128):
        super(ActorNetwork, self).__init__()
        periph_size = PERIPH_H * PERIPH_W  # 1107

        # Lean convolutional backbone
        self.conv1 = nn.Conv2d(1, 8, kernel_size=5, stride=1, padding=2) #1 input channel, 8 feature maps of size 27x41, 
        self.conv2 = nn.Conv2d(8, 16, kernel_size=3, stride=1, padding=1)
        self.conv3 = nn.Conv2d(16, 32, kernel_size=3, stride=1, padding=1)

        # Scalar embedding
        self.scalar_fc = nn.Linear(SCALAR_DIM, 16)

        # Compute flattened conv feature size after two 2x2 pools
        feat_h = PERIPH_H // 4  # 6
        feat_w = PERIPH_W // 4  # 10
        feat_dim = 32 * feat_h * feat_w  # 32*6*10 = 1920

        # Joint MLP
        self.fc1 = nn.Linear(feat_dim + 16, fc1_dims)
        self.fc2 = nn.Linear(fc1_dims, fc2_dims)

        # Action heads
        self.mu       = nn.Linear(fc2_dims, 2)
        self.sigma    = nn.Linear(fc2_dims, 2)
        self.discrete = nn.Linear(fc2_dims, 2)

        # Register max_action as buffer
        self.register_buffer('max_action', T.tensor(max_action, dtype=T.float32))

    def forward(self, state, dist=False):
        batch_size = state.size(0)
        periph_size = PERIPH_H * PERIPH_W

        # Split inputs
        grid    = state[:, :periph_size].view(batch_size, 1, PERIPH_H, PERIPH_W) # we want to treat these inputs as an image - for visualization, see /obs_debug
        scalars = state[:, periph_size:] # these we're going to treat as seperate inputs, since they arent part of the image but contain critical positional information


        #print(f"grid shape:   {grid.shape}")    # CONFIRMED SHAPE IS [1, 1, 27, 41]
        #print(f"scalars shape:{scalars.shape}")  # CONFIRMED SHAPE IS [1, 5]

        # Convolutional encoding
        x = F.relu(self.conv1(grid))
        #at this point, x is shape [batch_size, 8, 27, 41]
        x = F.relu(self.conv2(x))
        #at this point, x is shape [batch_size, 16, 27, 41]
        x = F.max_pool2d(x, 2)
        # at this point, x is shape [batch_size, 16, 13, 20]
        x = F.relu(self.conv3(x))
        # at this point, x is shape [batch_size, 32, 13, 20]
        x = F.max_pool2d(x, 2)
        grid_feats = x.view(batch_size, -1)

        # Scalar encoding
        s_feats = F.relu(self.scalar_fc(scalars))
        # s_feats is shape [batch_size, 16]

        # Joint MLP body
        joint = T.cat([grid_feats, s_feats], dim=1)
        # joint is shape [batch_size, 1936]
        h = F.relu(self.fc1(joint))
        # h is shape [batch_size, 256]
        h = F.relu(self.fc2(h))
        # h is shape [batch_size, 128]

        # Continuous distribution head
        mu        = self.mu(h)
        # mu is shape [batch_size, 2]
        sigma_raw = self.sigma(h)
        # sigma_raw is shape [batch_size, 2]
        sigma     = F.softplus(sigma_raw).clamp(MIN_SIGMA, MAX_SIGMA)
        base_dist = Normal(mu, sigma)
        tanh_dist = TransformedDistribution(base_dist, [TanhTransform(cache_size=1)])
        cont_dist = TransformedDistribution(
            tanh_dist,
            [AffineTransform(loc=0.0, scale=self.max_action.to(state.device))]
        )

        # Discrete distribution head
        disc_logits = self.discrete(h)
        # shape [batch_size, 2]
        disc_dist   = Categorical(logits=disc_logits)

        if dist:
            return cont_dist, disc_dist

        # Sample actions
        cont_action = cont_dist.rsample()
        disc_action = disc_dist.sample()
        return cont_action, disc_action

    def sample(self, state):
        cont_dist, disc_dist = self.forward(state, dist=True)
        cont = cont_dist.rsample()
        disc = disc_dist.sample()
        logp_c = cont_dist.log_prob(cont).sum(dim=-1)
        logp_d = disc_dist.log_prob(disc)
        return disc, cont, logp_d, logp_c

    def get_continuous_distribution(self, state):
        cont_dist, _ = self.forward(state, dist=True)
        return cont_dist

    def get_discrete_distribution(self, state):
        _, disc_dist = self.forward(state, dist=True)
        return disc_dist

    def save_checkpoint(self, path):
        T.save(self.state_dict(), path + '/actor.pth')

    def load_checkpoint(self, path):
        self.load_state_dict(T.load(path + '/actor.pth'))

class CriticNetwork(nn.Module):
    def __init__(self, lr, input_dims, fc1_dims=256, fc2_dims=128):
        super(CriticNetwork, self).__init__()
        periph_size = PERIPH_H * PERIPH_W

        # Lean convolutional backbone
        self.conv1 = nn.Conv2d(1, 8, kernel_size=5, stride=1, padding=2)
        self.conv2 = nn.Conv2d(8, 16, kernel_size=3, stride=1, padding=1)
        self.conv3 = nn.Conv2d(16, 32, kernel_size=3, stride=1, padding=1)

        # Scalar embedding
        self.scalar_fc = nn.Linear(SCALAR_DIM, 16)

        # Compute flattened conv feature size after two 2x2 pools
        feat_h = PERIPH_H // 4  # 6
        feat_w = PERIPH_W // 4  # 10
        feat_dim = 32 * feat_h * feat_w

        # Joint MLP for value
        self.fc1 = nn.Linear(feat_dim + 16, fc1_dims)
        self.fc2 = nn.Linear(fc1_dims, fc2_dims)
        self.v   = nn.Linear(fc2_dims, 1)

    def forward(self, state):
        batch_size = state.size(0)
        periph_size = PERIPH_H * PERIPH_W

        grid    = state[:, :periph_size].view(batch_size, 1, PERIPH_H, PERIPH_W)
        scalars = state[:, periph_size:]

        x = F.relu(self.conv1(grid))
        x = F.relu(self.conv2(x))
        x = F.max_pool2d(x, 2)
        x = F.relu(self.conv3(x))
        x = F.max_pool2d(x, 2)
        grid_feats = x.view(batch_size, -1)

        s_feats = F.relu(self.scalar_fc(scalars))
        joint   = T.cat([grid_feats, s_feats], dim=1)

        h = F.relu(self.fc1(joint))
        h = F.relu(self.fc2(h))
        value = self.v(h)
        return value

    def save_checkpoint(self, path):
        T.save(self.state_dict(), path + '/critic.pth')

    def load_checkpoint(self, path):
        self.load_state_dict(T.load(path + '/critic.pth'))