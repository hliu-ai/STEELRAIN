#include "ai/noisy_linear.hpp"
#include <cmath>

// Constructor: allocate and register parameters and buffers,
// then initialize parameters and noise.
NoisyLinearImpl::NoisyLinearImpl(int in_features, int out_features, bool use_bias, double std_init)
    : in_features(in_features),
      out_features(out_features),
      use_bias(use_bias),
      std_init(std_init)
{
    // Allocate and register weight parameters
    weight_mu = register_parameter("weight_mu", torch::empty({out_features, in_features}));
    weight_sigma = register_parameter("weight_sigma", torch::empty({out_features, in_features}));
    // Register a buffer for the noise (not optimized)
    weight_epsilon = register_buffer("weight_epsilon", torch::empty({out_features, in_features}));

    // Allocate and register bias parameters/buffer if requested
    if (use_bias) {
        bias_mu = register_parameter("bias_mu", torch::empty({out_features}));
        bias_sigma = register_parameter("bias_sigma", torch::empty({out_features}));
        bias_epsilon = register_buffer("bias_epsilon", torch::empty({out_features}));
    }

    reset_parameters();
    reset_noise();
}

// reset_parameters(): initialize μ and σ as described in the paper
void NoisyLinearImpl::reset_parameters() {
    torch::NoGradGuard no_grad; // Disable gradient tracking for in-place operations
    double mu_range = 1.0 / std::sqrt(in_features);
    weight_mu.uniform_(-mu_range, mu_range);
    weight_sigma.fill_(std_init / std::sqrt(in_features));
    if (use_bias) {
        bias_mu.uniform_(-mu_range, mu_range);
        bias_sigma.fill_(std_init / std::sqrt(out_features));
    }
}

// scale_noise(): sample noise from a standard normal and scale it as sign(x)*sqrt(|x|)
torch::Tensor NoisyLinearImpl::scale_noise(int size) {
    auto x = torch::randn({size}, torch::TensorOptions().device(weight_mu.device()));
    return x.sign() * torch::sqrt(x.abs());
}

// reset_noise(): resample the noise buffers for weights and bias using factorized Gaussian noise
void NoisyLinearImpl::reset_noise() {
    torch::NoGradGuard no_grad; // No gradient tracking during noise sampling
    auto epsilon_in = scale_noise(in_features);
    auto epsilon_out = scale_noise(out_features);
    // Outer product to produce weight noise of shape [out_features, in_features]
    weight_epsilon.copy_(epsilon_out.unsqueeze(1) * epsilon_in.unsqueeze(0));
    if (use_bias) {
        bias_epsilon.copy_(epsilon_out);
    }
}

// get_weight(): returns weight_mu + weight_sigma * weight_epsilon when training, otherwise weight_mu
torch::Tensor NoisyLinearImpl::get_weight() {
    if (this->is_training()) {
        return weight_mu + weight_sigma * weight_epsilon;
    } else {
        return weight_mu;
    }
}

// get_bias(): returns bias_mu + bias_sigma * bias_epsilon when training, otherwise bias_mu
torch::Tensor NoisyLinearImpl::get_bias() {
    if (use_bias) {
        if (this->is_training()) {
            return bias_mu + bias_sigma * bias_epsilon;
        } else {
            return bias_mu;
        }
    }
    return torch::Tensor();
}

// Forward pass: applies a linear transformation with the noisy parameters
torch::Tensor NoisyLinearImpl::forward(const torch::Tensor &input) {
    auto w = get_weight();
    auto b = get_bias();
    return torch::linear(input, w, b);
}
