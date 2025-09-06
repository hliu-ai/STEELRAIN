#include "experience_manager.hpp"
#include "player.hpp"
#include "ai/input_handler.hpp"
#include "ai/policy_network.hpp"
#include <iostream>

std::tuple<torch::Tensor, torch::Tensor, torch::Tensor, torch::Tensor, torch::Tensor>
ExperienceManager::create_experience(player_class& player,
                                      InputHandler& input_handler,
                                      PolicyNetwork& policy_network,
                                      float reward,
                                      bool done, torch::Tensor state_tensor, torch::Tensor action_tensor, torch::Tensor next_state_tensor) {


    

    
    // Create the reward tensor (r)
    torch::Tensor reward_tensor = torch::tensor({reward}, torch::kFloat32).view({1, 1}); // Shape [1, 1]
    if (torch::cuda::is_available()) {
        reward_tensor = reward_tensor.to(torch::kCUDA);
    }

    // Create the done tensor (d)
    torch::Tensor done_tensor = torch::tensor({done ? 1.0f : 0.0f}, torch::kFloat32).view({1, 1}); // Shape [1, 1]
    if (torch::cuda::is_available()) {
        done_tensor = done_tensor.to(torch::kCUDA);
    }

    //HOLD ON - HOW CAN WE BE SURE THAT THE NEXT FRAME OCCURED? I need to make some debugging outputs comparing (s) and (s') tensors here.
    //std::cout << "(From experience_manager.cpp) State: " << state_tensor << "\n";
    //std::cout << "Next State: " << next_state_tensor << "\n";
    


/*
    // Check if state_tensor and next_state_tensor are equal
    if (torch::equal(state_tensor, next_state_tensor)) {
        std::cout << "State tensor (s) and next state tensor (s') match." << std::endl;
    } else {
        std::cout << "State tensor (s) and next state tensor (s') DO NOT match." << std::endl;
    }
*/
    // Return the tuple containing (s, a, r, s', d)
    return {state_tensor, action_tensor, reward_tensor, next_state_tensor, done_tensor};
}



/*
    // Print the experience tuple for debugging
    std::cout << "Experience Tuple Shapes:\n";
    std::cout << "State Tensor Shape: " << state_tensor.sizes() << "\n";
    std::cout << "Action Tensor Shape: " << action_tensor.sizes() << "\n";
    std::cout << "Reward Tensor Shape: " << reward_tensor.sizes() << "\n";
    std::cout << "Next State Tensor Shape: " << next_state_tensor.sizes() << "\n";
    std::cout << "Done Tensor Shape: " << done_tensor.sizes() << "\n\n";

    
    std::cout << "Experience Tuple Values:\n";
    std::cout << "State: " << state_tensor << "\n";
    std::cout << "Action: " << action_tensor << "\n"; 
    std::cout << "Reward: " << reward_tensor << "\n"; //if you want to see the reward value raw, do reward_tensor.item<float>()
    std::cout << "Next State: " << next_state_tensor << "\n";
    std::cout << "Done: " << done_tensor << "\n";

    std::cout << "Device Information for Tensors:" << std::endl;
    std::cout << "State Tensor (s) Device: " << state_tensor.device() << std::endl;
    std::cout << "Action Tensor (a) Device: " << action_tensor.device() << std::endl;
    std::cout << "Reward Tensor (r) Device: " << reward_tensor.device() << std::endl;
    std::cout << "Next State Tensor (s') Device: " << next_state_tensor.device() << std::endl;
    std::cout << "Done Tensor (d) Device: " << done_tensor.device() << std::endl;
  */  