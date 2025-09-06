#include "policy_network.hpp"
#include "ai/noisy_linear.hpp"
#include <torch/torch.h>
#include <iostream>
#include <stdexcept>


// Constructor
PolicyNetwork::PolicyNetwork() {
    initialize_model();
}

// Initialize the model with noisy layers
void PolicyNetwork::initialize_model() {
    try {
        std::cout << "Initializing DQN model components with NoisyLinear layers..." << std::endl;

        // Register each layer as a module using NoisyLinear
        std::cout << "Creating Input Layer (122 -> 128) for DQN using NoisyLinear..." << std::endl;
        layer1 = register_module("layer1", NoisyLinear(122, 128, true, 0.1));
        std::cout << "Input Layer created." << std::endl;

        std::cout << "Creating Hidden Layer 1 (128 -> 64) for DQN using NoisyLinear..." << std::endl;
        layer2 = register_module("layer2", NoisyLinear(128, 64, true, 0.1));
        std::cout << "Hidden Layer 1 created." << std::endl;

        std::cout << "Creating Hidden Layer 2 (64 -> 32) for DQN using NoisyLinear..." << std::endl;
        layer3 = register_module("layer3", NoisyLinear(64, 32, true, 0.1));
        std::cout << "Hidden Layer 2 created." << std::endl;

        std::cout << "Creating Output Layer (32 -> 5) for DQN using NoisyLinear..." << std::endl;
        output_layer = register_module("output_layer", NoisyLinear(32, 5, true, 0.1));
        std::cout << "Output Layer created." << std::endl;

        // Check if CUDA is available and set model to GPU if possible
        if (torch::cuda::is_available()) {
            this->to(torch::Device(torch::kCUDA));
            std::cout << "Using CUDA for DQN model training and inference." << std::endl;
        } else {
            throw std::runtime_error("CUDA is not available. Terminating initialization.");
        }
    } catch (const std::exception &e) {
        std::cerr << "Failed to initialize DQN model: " << e.what() << std::endl;
        throw;
    }
}

// Forward pass of the DQN
torch::Tensor PolicyNetwork::forward(const torch::Tensor &inputs) {
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

// Save the model to a file
void PolicyNetwork::save_model(const std::string& path) {
    try {
        torch::serialize::OutputArchive archive;
        this->to(torch::kCPU);  // Move model to CPU for saving
        this->save(archive);
        archive.save_to(path);
        std::cout << "Model saved to " << path << std::endl;
        this->to(torch::kCUDA);  // Move model back to GPU if needed
    } catch (const std::exception& e) {
        std::cerr << "Failed to save the model: " << e.what() << std::endl;
    }
}


void PolicyNetwork::load_model(const std::string& path) {
    try {
        torch::serialize::InputArchive archive;
        archive.load_from(path);
        this->load(archive);
        this->to(torch::kCUDA);  // Move model to GPU for training/inference
        std::cout << "Model loaded successfully from: " << path << std::endl;

        // Ask the user for confirmation to proceed
        char user_response;
        std::cout << "Do you want to proceed with training using this model? (y/n): ";
        std::cin >> user_response;
        if (user_response != 'y' && user_response != 'Y') {
            std::cout << "Exiting program as per user request." << std::endl;
            std::exit(EXIT_SUCCESS); // Exit gracefully
        }

    } catch (const std::exception& e) {
        std::cerr << "Failed to load the model: " << e.what() << std::endl;
        std::cerr << "Model file not found or invalid." << std::endl;

        // Ask user if they want to proceed with a fresh model
        char user_response;
        std::cout << "Do you want to proceed with a fresh model? (y/n): ";
        std::cin >> user_response;
        if (user_response == 'y' || user_response == 'Y') {
            std::cout << "Proceeding with a fresh model." << std::endl;
        } else {
            std::cout << "Exiting program as per user request." << std::endl;
            std::exit(EXIT_FAILURE); // Exit if the user does not want to continue
        }
    }
}

void PolicyNetwork::reset_noise_all() {
    if (layer1) layer1->reset_noise();
    if (layer2) layer2->reset_noise();
    if (layer3) layer3->reset_noise();
    if (output_layer) output_layer->reset_noise();
}
