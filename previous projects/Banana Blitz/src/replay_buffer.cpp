#include "replay_buffer.hpp"
#include <algorithm>
#include <random>

// Constructor implementation
ReplayBuffer::ReplayBuffer(size_t capacity) : capacity(capacity) {
    // Initialize batched tensors as empty placeholders to avoid potential errors
    batched_states = torch::empty({0});
    batched_actions = torch::empty({0});
    batched_rewards = torch::empty({0});
    batched_next_states = torch::empty({0});
    batched_dones = torch::empty({0});
}

void ReplayBuffer::add_experience(const std::tuple<torch::Tensor, torch::Tensor, torch::Tensor, torch::Tensor, torch::Tensor>& experience) {
    // Extract the done tensor from the experience
    const torch::Tensor& done_tensor = std::get<4>(experience);

    // Check if the experience is terminal (done == 1)
    if (done_tensor.item<float>() == 1.0f) {
        //std::cout << "Terminal experience with done = 1 added to replay buffer!" << std::endl;
        //std::cout << "Done: " << done_tensor.item<float>() << std::endl;
    }

    // Maintain buffer capacity by removing the oldest experience if at capacity
    if (buffer.size() >= capacity) {
        buffer.pop_back();  // Remove the oldest experience
    }

    // Add the new experience to the front of the buffer
    buffer.push_front(experience);
}

std::vector<std::tuple<torch::Tensor, torch::Tensor, torch::Tensor, torch::Tensor, torch::Tensor>> ReplayBuffer::sample_batch(size_t batch_size) {
    std::vector<std::tuple<torch::Tensor, torch::Tensor, torch::Tensor, torch::Tensor, torch::Tensor>> sampled_batch;
    if (buffer.size() >= batch_size) {
        std::sample(buffer.begin(), buffer.end(), std::back_inserter(sampled_batch), batch_size, std::mt19937{std::random_device{}()});

        // Create batched tensors from the sampled experiences
        std::vector<torch::Tensor> states, actions, rewards, next_states, dones;
        for (const auto& exp : sampled_batch) {
            states.push_back(std::get<0>(exp));
            actions.push_back(std::get<1>(exp));
            rewards.push_back(std::get<2>(exp));
            next_states.push_back(std::get<3>(exp));
            dones.push_back(std::get<4>(exp));
        }

        batched_states = torch::cat(states, 0);
        batched_actions = torch::cat(actions, 0);
        batched_rewards = torch::cat(rewards, 0);
        batched_next_states = torch::cat(next_states, 0);
        batched_dones = torch::cat(dones, 0);
    }

    return sampled_batch;
}

bool ReplayBuffer::is_ready(size_t batch_size) const {
    return buffer.size() >= batch_size;
}

size_t ReplayBuffer::size() const {
    return buffer.size();
}

// Getters for batched tensors
torch::Tensor ReplayBuffer::get_batched_states() const {
    return batched_states;
}

torch::Tensor ReplayBuffer::get_batched_actions() const {
    return batched_actions;
}

torch::Tensor ReplayBuffer::get_batched_rewards() const {
    return batched_rewards;
}

torch::Tensor ReplayBuffer::get_batched_next_states() const {
    return batched_next_states;
}

torch::Tensor ReplayBuffer::get_batched_dones() const {
    return batched_dones;
}
