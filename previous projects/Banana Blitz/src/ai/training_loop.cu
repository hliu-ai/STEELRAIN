#include "training_loop.hpp"
#include "replay_buffer.hpp"
#include "ai/policy_network.hpp"
#include "ai/target_network.hpp"
#include "ai/input_handler.hpp"
#include <torch/torch.h>
#include <iostream>
#include <string>
#include <fstream>
#include <numeric>

    // Constructor implementation
TrainingLoop::TrainingLoop(float gamma, float learning_rate, PolicyNetwork& policy_network)
    : gamma(gamma),
      learning_rate(learning_rate),
      policy_network(policy_network), // Initialize policy network reference
      optimizer(policy_network.parameters(), torch::optim::AdamOptions(learning_rate)) { // Create optimizer
}

// Function to run one step of training
void TrainingLoop::train_step(
    TargetNetwork& target_network,
    ReplayBuffer& replay_buffer,
    size_t batch_size) {
    if (replay_buffer.size() < batch_size) {
        std::cout << "Not enough experiences in the replay buffer yet to sample!" << std::endl;
        return;
    }

    // Sample a batch from the replay buffer
    std::vector<std::tuple<torch::Tensor, torch::Tensor, torch::Tensor, torch::Tensor, torch::Tensor>> batch = replay_buffer.sample_batch(batch_size);
    // Extract batched tensors from the replay buffer and move them to CUDA
    torch::Tensor batched_states = replay_buffer.get_batched_states();
    torch::Tensor batched_actions = replay_buffer.get_batched_actions();
    torch::Tensor batched_rewards = replay_buffer.get_batched_rewards();
    torch::Tensor batched_next_states = replay_buffer.get_batched_next_states();
    torch::Tensor batched_dones = replay_buffer.get_batched_dones();


    // Reset noise in the policy network before the forward pass
    policy_network.reset_noise_all();
    // Forward pass through the policy network for current Q-values
    this->batched_q_values = policy_network.forward(batched_states);
    this->batched_q_values_taken = batched_q_values.gather(1, batched_actions.to(torch::kLong));

    // Reset noise in the target network before computing the target Q-values
    target_network.reset_noise_all();
    // Forward pass through the target network to get next Q-values (detach to remove from computation graph)
    this->batched_next_q_targets = target_network.forward(batched_next_states).detach();
    
    // Forward pass through the policy network for next state Q-values (used for choosing the best action)
    //policy_network.reset_noise_all();  // Optionally refresh noise again if desired - i wont here.
    this->batched_next_q_policy = policy_network.forward(batched_next_states);
    this->batched_next_actions = std::get<1>(batched_next_q_policy.max(1, true));
    this->batched_selected_q_next_values = batched_next_q_targets.gather(1, batched_next_actions);

    // Bellman Equation: compute target Q-values
    this->target_q_values = batched_rewards + (gamma * batched_selected_q_next_values * (1 - batched_dones));

    // Compute the loss using MSE between current and target Q-values
    this->loss = torch::mse_loss(batched_q_values_taken, target_q_values);

    optimizer.zero_grad();  // Clear old gradients
    loss.backward();        // Compute gradients via backpropagation
    optimizer.step();       // Update the policy network's weights

    // Update the loss buffer for logging
    loss_buffer.push_back(loss.item<float>());
    if (loss_buffer.size() > max_loss_buffer_size) {
        loss_buffer.pop_front();
    }
}

    /*
    // Print debug information
    std::cout << "Training step completed. Loss: " << loss.item<float>() << std::endl;
    std::cout << "Loss Buffer: ";
    for (const auto& loss : loss_buffer) {
        std::cout << loss << " ";
    }
    std::cout << std::endl;
    std::cout << "Average Loss: " << get_average_loss() << std::endl;
    */

void TrainingLoop::save_loss_to_csv(const std::string& file_path, size_t step) const {
    std::ofstream file(file_path, std::ios::app);  // Open file in append mode
    if (file.is_open()) {
        // Calculate average loss from the rolling buffer
        float average_loss = std::accumulate(loss_buffer.begin(), loss_buffer.end(), 0.0f) / loss_buffer.size();

        // Write the current step and averaged loss to CSV
        file << step << "," << average_loss << "\n";
    } else {
        std::cerr << "Error: Could not open file for writing loss data.\n";
    }
}


// Getter implementations
torch::Tensor TrainingLoop::get_batched_next_q_targets() const {
    return batched_next_q_targets;
}

torch::Tensor TrainingLoop::get_batched_next_q_policy() const {
    return batched_next_q_policy;
}

torch::Tensor TrainingLoop::get_batched_next_actions() const {
    return batched_next_actions;
}

torch::Tensor TrainingLoop::get_batched_selected_q_next_values() const {
    return batched_selected_q_next_values;
}

torch::Tensor TrainingLoop::get_batched_q_values() const {
    return batched_q_values;
}

torch::Tensor TrainingLoop::get_batched_q_values_taken() const {
    return batched_q_values_taken;
}

torch::Tensor TrainingLoop::get_target_q_values() const {
    return target_q_values;
}

torch::Tensor TrainingLoop::get_loss() const {
    return loss;
}

float TrainingLoop::get_average_loss() const {
    if (loss_buffer.empty()) {
        return 0.0f; //no loss values recorded yet
    }
    return std::accumulate(loss_buffer.begin(), loss_buffer.end(), 0.0f) / loss_buffer.size();
}

const std::deque<float>& TrainingLoop::get_recent_losses() const {
    return loss_buffer; //provide access to the raw loss values
}

void TrainingLoop::copy_weights_from_policy_to_target(PolicyNetwork& policy_network,
                                                      TargetNetwork& target_network,
                                                      InputHandler& input_handler,
                                                      int global_frame_counter,
                                                      int copy_interval) {
    // Check if it's time to copy weights
    if (global_frame_counter % copy_interval == 0) {
        // Copy weights
        target_network.copy_weights_from(policy_network);
        std::cout << "Target network updated at frame interval: " << global_frame_counter << "." << std::endl;

        // Generate debugging input tensor
        torch::Tensor debugging_input_tensor = input_handler.generate_debug_input();

        // Pass through both networks
        torch::Tensor policy_output = policy_network.forward(debugging_input_tensor);
        torch::Tensor target_output = target_network.forward(debugging_input_tensor);

        // Debugging output
        std::cout << "Policy Output: " << policy_output << std::endl;
        std::cout << "Target Output: " << target_output << std::endl;

        // Verify if weights match
        if (target_network.compare_parameters(policy_network, target_network)) {
            std::cout << "Target network successfully copied from policy network." << std::endl;
        } else {
            std::cerr << "Error: Target network parameters do not match the policy network." << std::endl;
        }

        // Verify if outputs match
        if (torch::allclose(policy_output, target_output, 1e-5, 1e-8)) {
            std::cout << "Outputs match between the policy network and the target network." << std::endl;
        } else {
            std::cerr << "Outputs do NOT match between the policy network and the target network." << std::endl;
        }
    }
}

//used to be based on an interval, now it just does what is asked.
void TrainingLoop::save_model(PolicyNetwork& policy_network,
                              const std::string& save_dir,
                              const std::string& run_id,
                              int global_frame_counter) {

    std::string save_path = save_dir + "/policy_network_" + run_id + "_" + std::to_string(global_frame_counter) + ".pt";

    // Save the model
    policy_network.save_model(save_path);

    // Log the save action
    std::cout << "Model saved at frame: " << global_frame_counter << " to " << save_path << std::endl;
}


void TrainingLoop::train_if_ready(int global_frame_counter, int training_interval, ReplayBuffer& replay_buffer,
                                  int batch_size, TargetNetwork& target_network) {
    // Check if it's time to train and if the replay buffer is ready
    if (global_frame_counter % training_interval == 0 && replay_buffer.is_ready(batch_size)) {
        train_step(target_network, replay_buffer, batch_size); // Perform the training step
    }
}