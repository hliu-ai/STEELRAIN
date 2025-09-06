#include "input_handler.hpp"
#include "object_type.hpp"
#include "raycasting.hpp"
#include "proximity_radius.hpp"
#include "player.hpp" // Include player header
#include <torch/torch.h>
#include <algorithm> // for padding or additional utility
#include <cmath> // for distance calculations
#include <cassert>
#include <cstdlib> //for rand() and RAND_MAX
#include <ctime>   // For seeding random numbers
#include <iostream>

InputHandler::InputHandler() {
    // Initialize any required data if necessary
}

//Note: this input handler handles both "inputs" in the sense that the player is inputting a move and getting an action performed back and also "inputs" in the sense of what mr. monkey's organs tell him about the world. I just got lazy and didn't want to make another file because i wouldnt know what to call it.


void InputHandler::handle_actions(player_class &player, float delta_time, const std::vector<environmental_object>& platforms, const std::vector<std::unique_ptr<collectable>>& bananas, float acceleration_factor) {
    bool player_up = false, player_down = false, player_left = false, player_right = false;

    if (is_ai_controlled) {
        // Get the action index from the policy network (assume this is passed or stored globally)
        int action_index = get_current_action_index(); // This is set by the DQN logic in the main loop - from experience_manager -> 

        // Map the action index to specific movement
        switch (action_index) {
            case 0: // Idle
                player_up = false;
                player_down = false;
                player_left = false;
                player_right = false;
                break;
            case 1: // Move up
                player_up = true;
                break;
            case 2: // Move down
                player_down = true;
                break;
            case 3: // Move left
                player_left = true;
                break;
            case 4: // Move right
                player_right = true;
                break;
            default:
                std::cerr << "Invalid action index: " << action_index << std::endl;
                break;
        }
    } else {
        // Human control mode: handle keyboard input
        player_up = sf::Keyboard::isKeyPressed(sf::Keyboard::Up);
        player_down = sf::Keyboard::isKeyPressed(sf::Keyboard::Down);
        player_left = sf::Keyboard::isKeyPressed(sf::Keyboard::Left);
        player_right = sf::Keyboard::isKeyPressed(sf::Keyboard::Right);
    }

    // Update the player's position based on the inputs
    player.update(player_up, player_down, player_left, player_right, delta_time, platforms, bananas, acceleration_factor);
}






// Setter for action index
void InputHandler::set_current_action_index(int index, float current_epsilon) {
    int post_epsilon_action = apply_epsilon(index, current_epsilon); // Apply epsilon-greedy logic
    action_index = post_epsilon_action; // Store the final action index after exploration consideration
    update_epsilon(); // Update epsilon after choosing the action
}

// Apply epsilon to the index
int InputHandler::apply_epsilon(int index, float current_epsilon) {
    // Generate a random float between 0 and 1 to decide if exploration occurs
    if (static_cast<float>(rand()) / static_cast<float>(RAND_MAX) < current_epsilon) {
        // Exploration: return a random action (0 to 4 if 5 total actions)
        return std::rand() % 5;
    } else {
        // Exploitation: return the action with the highest Q-value (the provided index)
        return index;
    }
}


void InputHandler::update_epsilon() {
// Decay epsilon but ensure it doesn’t fall below a minimum value, like 0.01
    current_epsilon = std::max(0.0f, current_epsilon - fixed_epsilon_decay);
    //std::cout << "Current epsilon is: " << current_epsilon << std::endl;
}

float InputHandler::get_current_epsilon() const {
    return current_epsilon;
}

// Getter for action index
int InputHandler::get_current_action_index() const {
    return action_index;
}


void InputHandler::set_ai_controlled(bool is_ai) {
    is_ai_controlled = is_ai;
}

// Function to gather raycasting data
std::vector<float> InputHandler::get_ray_inputs(const Raycasting &raycasting) {
    std::vector<float> ray_data;

    // Retrieve ray collisions and rays
    const auto& ray_collisions = raycasting.get_ray_collisions();
    const auto& rays = raycasting.get_rays();

    for (size_t i = 0; i < rays.size(); ++i) { // for all 20 rays
        const auto& ray = rays[i];
        float distance = std::hypot(ray[1].position.x - ray[0].position.x, ray[1].position.y - ray[0].position.y);

        // Get the collision type from the ray_collisions vector
        ObjectType collision_type = ray_collisions[i];

        // Push ray data: distance, collision type, endpoint x, endpoint y. This represents the 4 in the 4*20 inputs.
        ray_data.push_back(distance);
        ray_data.push_back(static_cast<float>(collision_type)); // Encode object type numerically. Values are in object_type.hpp, 0 for none, 1 for banana, 2 for platform
        ray_data.push_back(ray[1].position.x); // endpoint x
        ray_data.push_back(ray[1].position.y); // endpoint y
    }
    return ray_data;
}

// Function to gather proximity radius data
std::vector<float> InputHandler::get_proximity_inputs(const ProximityRadius &proximity_radius) {
    std::vector<float> proximity_data;

    for (const auto &object : proximity_radius.get_nearby_objects()) {
        proximity_data.push_back(object.distance);
        proximity_data.push_back(static_cast<float>(object.interaction_type)); // Use ObjectType directly as a float value
        proximity_data.push_back(object.nearest_point.x);
        proximity_data.push_back(object.nearest_point.y);
    }

    pad_proximity_data(proximity_data); // Pad if there are fewer objects than expected

    return proximity_data;
}

// Function to pad proximity data if fewer objects are detected
void InputHandler::pad_proximity_data(std::vector<float> &proximity_data) {
    constexpr int max_slots = 10; // Maximum number of proximity slots
    constexpr int features_per_slot = 4;

    while (proximity_data.size() < max_slots * features_per_slot) {
        // Pad with -1.0f to maintain fixed size, representing no nearby object
        proximity_data.push_back(-1.0f); // distance
        proximity_data.push_back(-1.0f); // interaction type
        proximity_data.push_back(-1.0f); // nearest_point.x
        proximity_data.push_back(-1.0f); // nearest_point.y
    }
}

// Function to gather player-specific inputs we don't need the whole object, just the position
std::vector<float> InputHandler::get_player_inputs(const player_class &player) {
    // Create a vector to store player-specific data
    std::vector<float> player_data;

    // Extract the x and y positions from the player using the getter methods
    player_data.push_back(player.get_x_position());
    player_data.push_back(player.get_y_position());

    return player_data;
}

// Function to normalize proximity radius inputs
void InputHandler::normalize_proximity_radius_inputs(std::vector<float> &proximity_data) {
    constexpr int features_per_slot = 4;
    for (size_t i = 0; i < proximity_data.size(); i += features_per_slot) {
        if (proximity_data[i] != -1.0f) { // Normalize only if it's not a padded value
            // Normalize distance assuming max possible distance is the radius itself
            proximity_data[i] /= 150.0f; // Assuming 150 is the maximum radius

            // Normalize interaction type 0 = None, 1 = Banana, 2 = Platform
            proximity_data[i + 1] /= 2.0f;

            // Normalize nearest_point.x and nearest_point.y assuming window size is 1000x800
            proximity_data[i + 2] /= 1000.0f;
            proximity_data[i + 3] /= 800.0f;
        }
    }
}

void InputHandler::normalize_raycasting_inputs(std::vector<float> &ray_data) {
    constexpr int features_per_ray = 4;
    
    // Ensure that the ray_data is a multiple of features_per_ray
    assert(ray_data.size() % features_per_ray == 0 && "Ray data size is not a multiple of features_per_ray");

    for (size_t i = 0; i < ray_data.size(); i += features_per_ray) {
        if (ray_data[i] != -1.0f) { // Normalize only if it's not a padded value
            // Normalize distance (assuming max ray length is 1500
            ray_data[i] /= 1500.0f;

            // Normalize collision type (0 = None, 1 = Banana, 2 = Platform
            ray_data[i + 1] /= 2.0f;

            // Normalize endpoint x and y (assuming window size is 1000x800
            ray_data[i + 2] /= 1000.0f;
            ray_data[i + 3] /= 800.0f;
        }
    }
}

// Function to normalize player-specific inputs
void InputHandler::normalize_player_inputs(std::vector<float> &player_data) {
    // Normalize player x and y positions (assuming window size is 1000x800
    player_data[0] /= 1000.0f;
    player_data[1] /= 800.0f;
}

// Convert combined input vector to a Torch tensor
torch::Tensor InputHandler::convert_to_tensor(const std::vector<float>& combined_inputs) {
    // Convert std::vector<float> to torch::Tensor and move to GPU
    torch::Tensor tensor = torch::from_blob(
        const_cast<float*>(combined_inputs.data()), 
        {1, static_cast<long>(combined_inputs.size())}, 
        torch::kFloat
    ).clone().to(torch::kCUDA); // clone to copy data since from_blob just references the original, and move to GPU

    return tensor;
}

// Function to combine all inputs and directly convert to a Torch tensor
torch::Tensor InputHandler::get_combined_inputs(const Raycasting &raycasting, const ProximityRadius &proximity_radius, const player_class &player) {
    // Gather inputs from different parts of the game environment
    std::vector<float> ray_data = get_ray_inputs(raycasting);
    std::vector<float> proximity_data = get_proximity_inputs(proximity_radius);
    std::vector<float> player_data = get_player_inputs(player);

    // Normalize each set of inputs
    normalize_raycasting_inputs(ray_data);
    normalize_proximity_radius_inputs(proximity_data);
    normalize_player_inputs(player_data);

    // Combine all inputs into one vector
    std::vector<float> combined_inputs;
    combined_inputs.reserve(ray_data.size() + proximity_data.size() + player_data.size());  // Reserve space to improve efficiency

    combined_inputs.insert(combined_inputs.end(), ray_data.begin(), ray_data.end());
    combined_inputs.insert(combined_inputs.end(), proximity_data.begin(), proximity_data.end());
    combined_inputs.insert(combined_inputs.end(), player_data.begin(), player_data.end());

    // Set negative values to zero only if they were introduced due to a calculation error (not for padding values
    for (auto &value : combined_inputs) {
        if (value < 0.0f && value != -1.0f) {
            value = 0.0f;
        }
    }

    // Convert combined input vector to a Torch tensor and move to GPU
    torch::Tensor input_tensor = torch::from_blob(
        combined_inputs.data(),
        {1, static_cast<long>(combined_inputs.size())},
        torch::kFloat
    ).clone().to(torch::kCUDA); // clone to copy data since from_blob just references the original, and move to GPU
    
    /*
    // Debugging output to check if the tensor is on GPU
    if (input_tensor.device().is_cuda()) {
        std::cout << "Tensor is on GPU." << std::endl;
    } else {
        std::cout << "Tensor is on CPU. CUDA move failed." << std::endl;
    }
    */
    return input_tensor;
}


torch::Tensor InputHandler::generate_debug_input() {
    // Generate a random tensor with the same shape as your state input
    torch::Tensor debug_tensor = torch::rand({1, 122}, torch::kFloat32); // Adjust size to match your state tensor
    if (torch::cuda::is_available()) {
        debug_tensor = debug_tensor.to(torch::kCUDA);
    }
    return debug_tensor;
}
