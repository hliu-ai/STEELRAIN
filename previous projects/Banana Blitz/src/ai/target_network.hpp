#ifndef TARGET_NETWORK_HPP
#define TARGET_NETWORK_HPP

#include <torch/torch.h>
#include <torch/script.h>
#include <ai/policy_network.hpp>
#include "ai/noisy_linear.hpp"  // Include the custom noisy layer
#include <string>

class TargetNetwork : public torch::nn::Module {
public:
    TargetNetwork();
    void initialize_model();
    torch::Tensor forward(const torch::Tensor& inputs);
    void copy_weights_from(const torch::nn::Module& policy_network);
    void copy_weights_from(const PolicyNetwork& policy_network);

    // Debugging function to compare parameters between two networks
    bool compare_parameters(const torch::nn::Module& net1, const torch::nn::Module& net2) const;

    void save_model(const std::string& path);
    void load_model(const std::string& path);

    // Helper function to reset noise in all noisy layers
    void reset_noise_all();

private:
    // Replace standard Linear layers with our custom NoisyLinear layers
    NoisyLinear layer1{nullptr};
    NoisyLinear layer2{nullptr};
    NoisyLinear layer3{nullptr};
    NoisyLinear output_layer{nullptr};
};

#endif // TARGET_NETWORK_HPP
