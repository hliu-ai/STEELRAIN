#ifndef EXPERIENCE_MANAGER_HPP
#define EXPERIENCE_MANAGER_HPP

#include <torch/torch.h>
#include <tuple>

class ExperienceManager {
public:
    // Constructor (if needed)
    ExperienceManager() = default;

    // Function to create and return an experience tuple (s, a, r, s', d)
    std::tuple<torch::Tensor, torch::Tensor, torch::Tensor, torch::Tensor, torch::Tensor>
    create_experience(class player_class& player,
                      class InputHandler& input_handler,
                      class PolicyNetwork& policy_network,
                      float reward,
                      bool done, torch::Tensor state_tensor, torch::Tensor action_tensor, torch::Tensor next_state_tensor);

};

#endif // EXPERIENCE_MANAGER_HPP

