#include "target_network.hpp"
#include "ai/noisy_linear.hpp"
#include <torch/torch.h>
#include <torch/script.h>
#include <iostream>
#include <stdexcept>

// Constructor
TargetNetwork::TargetNetwork() {
    initialize_model();
}

// Initialize the target network model with the same noisy layers architecture as the policy network
void TargetNetwork::initialize_model() {
    try {
        std::cout << "Initializing Target Network components with NoisyLinear layers..." << std::endl;

        // Register each layer using NoisyLinear
        layer1 = register_module("layer1", NoisyLinear(122, 128, true, 0.1));
        layer2 = register_module("layer2", NoisyLinear(128, 64, true, 0.1));
        layer3 = register_module("layer3", NoisyLinear(64, 32, true, 0.1));
        output_layer = register_module("output_layer", NoisyLinear(32, 5, true, 0.1));

        // Check if CUDA is available and move the network to GPU
        if (torch::cuda::is_available()) {
            this->to(torch::Device(torch::kCUDA));
            std::cout << "Using CUDA for Target Network inference." << std::endl;
        } else {
            throw std::runtime_error("CUDA is not available. Target Network will not use GPU.");
        }
    } catch (const std::exception& e) {
        std::cerr << "Failed to initialize Target Network: " << e.what() << std::endl;
        throw;
    }
}

torch::Tensor TargetNetwork::forward(const torch::Tensor &inputs) {
    try {
        torch::Tensor x = torch::relu(layer1->forward(inputs));
        x = torch::relu(layer2->forward(x));
        x = torch::relu(layer3->forward(x));
        x = output_layer->forward(x);
        return x;
    } catch (const std::exception &e) {
        std::cerr << "Failed during forward pass: " << e.what() << std::endl;
        throw;
    }
}

void TargetNetwork::copy_weights_from(const PolicyNetwork& policy_network) {
    try {
        torch::NoGradGuard no_grad;  // Disable gradient tracking

        // Retrieve parameters and buffers from both networks
        auto params1 = this->named_parameters(true /*recurse*/);
        auto buffers1 = this->named_buffers(true /*recurse*/);

        auto params2 = policy_network.named_parameters(true /*recurse*/);
        auto buffers2 = policy_network.named_buffers(true /*recurse*/);

        // Copy each parameter (the underlying μ and σ values are copied exactly)
        for (const auto& p : params2) {
            const std::string& name = p.key();
            if (params1.contains(name)) {
                params1[name].copy_(p.value().to(params1[name].device()));
            } else {
                std::cerr << "Parameter " << name << " not found in the target network." << std::endl;
            }
        }

        // Copy buffers (e.g., noise buffers)
        for (const auto& b : buffers2) {
            const std::string& name = b.key();
            if (buffers1.contains(name)) {
                buffers1[name].copy_(b.value().to(buffers1[name].device()));
            } else {
                std::cerr << "Buffer " << name << " not found in the target network." << std::endl;
            }
        }

        std::cout << "Weights copied successfully from the policy network to the target network on GPU." << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error copying weights: " << e.what() << std::endl;
    }
}

void TargetNetwork::copy_weights_from(const torch::nn::Module& policy_network) {
    // Forward the call to the overload taking a PolicyNetwork
    copy_weights_from(dynamic_cast<const PolicyNetwork&>(policy_network));
}

bool TargetNetwork::compare_parameters(const torch::nn::Module& net1, const torch::nn::Module& net2) const {
    auto params1 = net1.named_parameters();
    auto params2 = net2.named_parameters();

    for (const auto& p1 : params1) {
        const auto& name = p1.key();
        if (params2.contains(name)) {
            if (!torch::allclose(p1.value(), params2[name])) {
                std::cerr << "Mismatch found in parameter: " << name << std::endl;
                return false;
            }
        } else {
            std::cerr << "Parameter " << name << " not found in the second network." << std::endl;
            return false;
        }
    }
    std::cout << "All parameters match between the two networks." << std::endl;
    return true;
}

void TargetNetwork::save_model(const std::string& path) {
    try {
        auto state_dict = this->named_parameters();
        torch::serialize::OutputArchive archive;
        for (const auto& p : state_dict) {
            archive.write(p.key(), p.value());
        }
        archive.save_to(path);
        std::cout << "Model parameters saved to " << path << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Failed to save the model: " << e.what() << std::endl;
    }
}

void TargetNetwork::load_model(const std::string& path) {
    try {
        torch::serialize::InputArchive archive;
        archive.load_from(path);
        auto state_dict = this->named_parameters();
        for (auto& p : state_dict) {
            archive.read(p.key(), p.value());
        }
        this->to(torch::kCUDA);
        std::cout << "Model parameters loaded from " << path << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Failed to load the model: " << e.what() << std::endl;
    }
}

void TargetNetwork::reset_noise_all() {
    if (layer1) layer1->reset_noise();
    if (layer2) layer2->reset_noise();
    if (layer3) layer3->reset_noise();
    if (output_layer) output_layer->reset_noise();
}