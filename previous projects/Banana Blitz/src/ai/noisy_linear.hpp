#ifndef NOISY_LINEAR_HPP
#define NOISY_LINEAR_HPP

#include <torch/torch.h>

// Custom NoisyLinear layer implementation
class NoisyLinearImpl : public torch::nn::Module {
public:
    // Constructor
    // in_features: number of input features
    // out_features: number of output features
    // use_bias: whether to include a bias term (default true)
    // std_init: initial standard deviation for noise (default 0.1)
    NoisyLinearImpl(int in_features, int out_features, bool use_bias = true, double std_init = 0.1);

    // Forward pass
    torch::Tensor forward(const torch::Tensor &input);

    // Reset the noise buffers (should be called as needed during training)
    void reset_noise();

    // Reset the layer's parameters (called during construction)
    void reset_parameters();

    // Get the effective weight and bias (noise added when in training mode)
    torch::Tensor get_weight();
    torch::Tensor get_bias();

private:
    // Helper function: scales noise by f(x) = sign(x)*sqrt(|x|)
    torch::Tensor scale_noise(int size);

    int in_features;
    int out_features;
    bool use_bias;
    double std_init;

    // Learnable parameters
    torch::Tensor weight_mu;
    torch::Tensor weight_sigma;
    torch::Tensor bias_mu;
    torch::Tensor bias_sigma;

    // Buffers for noise (non-learnable)
    torch::Tensor weight_epsilon;
    torch::Tensor bias_epsilon;
};

TORCH_MODULE(NoisyLinear); // Creates a module holder for NoisyLinearImpl

#endif // NOISY_LINEAR_HPP
