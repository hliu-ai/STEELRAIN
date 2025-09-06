#ifndef INPUT_HANDLER_HPP
#define INPUT_HANDLER_HPP

#include "raycasting.hpp"
#include "proximity_radius.hpp"
#include "player.hpp"
#include <torch/torch.h>
#include <vector>
#include <SFML/Graphics.hpp>

class InputHandler {
public:
    InputHandler(); // Constructor


    // Mode management
    void set_ai_controlled(bool is_ai);

    int get_sticky_action (int index);

        // Getter for action index
    int get_current_action_index() const;
        // Setter for action index
    void set_current_action_index(int index, float current_epsilon);

    float get_current_epsilon() const;

     // Handle player actions
    void handle_actions(player_class &player, float delta_time, const std::vector<environmental_object>& platforms, const std::vector<std::unique_ptr<collectable>>& bananas, float acceleration_factor);

    // Gather raycasting input data
    std::vector<float> get_ray_inputs(const Raycasting &raycasting); //returns ray_data, which holds the 4*20 inputs.

    // Gather proximity radius input data
    std::vector<float> get_proximity_inputs(const ProximityRadius &proximity_radius);

    // Gather player-specific input data
    std::vector<float> get_player_inputs(const player_class &player);

    // Combined input for feeding to the AI
    torch::Tensor get_combined_inputs(const Raycasting &raycasting, const ProximityRadius &proximity_radius, const player_class &player);

      // New method to convert combined inputs to a tensor
    torch::Tensor convert_to_tensor(const std::vector<float>& combined_inputs);

    torch::Tensor generate_debug_input();

private:
    bool is_ai_controlled;
    int action_index = 0; // Default to idle - first move will always be idle.
    // Utility function that will be used inside get_proximity_inputs
    void pad_proximity_data(std::vector<float> &proximity_data); //Needed to ensure functionality for the ten "slots" reserved for possible thick rays
    void normalize_raycasting_inputs(std::vector<float> &ray_inputs);
    void normalize_proximity_radius_inputs(std::vector<float> &proximity_inputs);
    void normalize_player_inputs(std::vector<float> &player_inputs);


    int apply_epsilon(int index, float current_epsilon);
    void update_epsilon();

    float current_epsilon = 0.0f;
    float fixed_epsilon_decay = 0.00000f;

    
    int last_action_index = -1; //Stores the last action index chosen by the policy network
    
};

#endif // INPUT_HANDLER_HPP
