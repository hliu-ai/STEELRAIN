#ifndef POLICY_NETWORK_HPP
#define POLICY_NETWORK_HPP

#include <torch/torch.h>
#include <string>
#include "ai/noisy_linear.hpp"  // Include our custom layer

class PolicyNetwork : public torch::nn::Module {
public:
    PolicyNetwork();
    torch::Tensor forward(const torch::Tensor &inputs);
    void save_model(const std::string& path);
    void load_model(const std::string& path);

    // Replace all layers with NoisyLinear layers for exploration
    NoisyLinear layer1{nullptr};
    NoisyLinear layer2{nullptr};
    NoisyLinear layer3{nullptr};
    NoisyLinear output_layer{nullptr};

    // Helper function to reset noise in all noisy layers
    void reset_noise_all();

private:
    void initialize_model();
};

#endif // POLICY_NETWORK_HPP
