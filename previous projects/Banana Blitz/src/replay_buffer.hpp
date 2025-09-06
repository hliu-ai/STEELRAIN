#ifndef REPLAY_BUFFER_HPP
#define REPLAY_BUFFER_HPP

#include <deque>
#include <tuple>
#include <torch/torch.h>

class ReplayBuffer {
public:
    // Constructor declaration
    ReplayBuffer(size_t capacity);

    void add_experience(const std::tuple<torch::Tensor, torch::Tensor, torch::Tensor, torch::Tensor, torch::Tensor>& experience);
    std::vector<std::tuple<torch::Tensor, torch::Tensor, torch::Tensor, torch::Tensor, torch::Tensor>> sample_batch(size_t batch_size);
    bool is_ready(size_t batch_size) const;
    size_t size() const;

    // Getters for batched tensors
    torch::Tensor get_batched_states() const;
    torch::Tensor get_batched_actions() const;
    torch::Tensor get_batched_rewards() const;
    torch::Tensor get_batched_next_states() const;
    torch::Tensor get_batched_dones() const;

private:
    std::deque<std::tuple<torch::Tensor, torch::Tensor, torch::Tensor, torch::Tensor, torch::Tensor>> buffer;
    size_t capacity;

    // Batched tensors
    torch::Tensor batched_states;
    torch::Tensor batched_actions;
    torch::Tensor batched_rewards;
    torch::Tensor batched_next_states;
    torch::Tensor batched_dones;
};

#endif // REPLAY_BUFFER_HPP
