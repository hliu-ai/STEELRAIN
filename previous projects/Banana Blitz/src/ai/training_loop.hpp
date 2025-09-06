#ifndef TRAINING_LOOP_HPP
#define TRAINING_LOOP_HPP

#include <torch/torch.h>
#include "ai/policy_network.hpp"
#include "ai/target_network.hpp"
#include "ai/input_handler.hpp"
#include "replay_buffer.hpp"
#include <string>
#include <deque>
#include <numeric>

class TrainingLoop {
public:
    // Constructor
    TrainingLoop(float gamma, float learning_rate, PolicyNetwork& policy_network);

    
    
    // Training step function
    void train_step(
        TargetNetwork& target_network,
        ReplayBuffer& replay_buffer,
        size_t batch_size
    );

        // Getters for debugging
    torch::Tensor get_batched_next_q_targets() const;
    torch::Tensor get_batched_next_q_policy() const;
    torch::Tensor get_batched_next_actions() const;
    torch::Tensor get_batched_selected_q_next_values() const;
    torch::Tensor get_batched_q_values() const;
    torch::Tensor get_batched_q_values_taken() const;
    torch::Tensor get_target_q_values() const;
    torch::Tensor get_loss() const;

    //copy weights over
    void copy_weights_from_policy_to_target(PolicyNetwork& policy_network,
                                        TargetNetwork& target_network,
                                        InputHandler& input_handler,
                                        int global_frame_counter,
                                        int copy_interval);

    void save_model(PolicyNetwork& policy_network, const std::string& save_dir, const std::string& run_id, int global_frame_counter);

    void train_if_ready(int global_frame_counter, int training_interval, ReplayBuffer& replay_buffer,
                    int batch_size, TargetNetwork& target_network);
                    
    //For loss graph
    float get_average_loss() const; //Returns the average of the current loss buffer
    const std::deque<float>& get_recent_losses() const; //returns the raw buffer

    void save_loss_to_csv(const std::string& file_path, size_t step) const;

private:
    float gamma = 0.99;  // Discount factor for the Bellman equation
    float learning_rate;
    PolicyNetwork& policy_network;
    torch::optim::Adam optimizer;

    // Member variables for debugging
    torch::Tensor batched_next_q_targets;
    torch::Tensor batched_next_q_policy;
    torch::Tensor batched_next_actions;
    torch::Tensor batched_selected_q_next_values;
    torch::Tensor batched_q_values;
    torch::Tensor batched_q_values_taken;
    torch::Tensor target_q_values;
    torch::Tensor loss;

    //For loss graph
    std::deque<float> loss_buffer; //sliding window for recent loss values
    const size_t max_loss_buffer_size = 1000; //adjustable buffer size
    

    
};

#endif // TRAINING_LOOP_HPP
