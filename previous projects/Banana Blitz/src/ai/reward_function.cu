#include "reward_function.hpp"
#include "player.hpp"
#include "environmental_object.hpp"
#include "collectable.hpp"
#include <iostream>
#include <cmath> 

RewardFunction::RewardFunction() {
    // Initialize previous positions to an invalid value, like -1, to detect the first frame
    prev_x_position = -1.0f;
    prev_y_position = -1.0f;

}

// Calculate rewards based on the current state of the game environment. Called every frame - is thus reset to 0 at the top of the function.
//Debugging outputs are included - comment in or out as needed.
float RewardFunction::calculate_reward(const player_class &player, int bananas_collected, float delta_time, bool level_completed, bool time_exceeded, float acceleration_factor) {
    float reward = 0.0f;

    float adjusted_delta_time = delta_time * acceleration_factor;


    // Penalty for Time Elapsing - To help incentize speed. Linear.
    float time_penalty = adjusted_delta_time * 1.0f;  // 1.0 points per second loss
    reward -= time_penalty;
    //std::cout << "Time penalty applied: -" << time_penalty << " points" << std::endl;

    // Reward for Banana Collection...
    if (bananas_collected > 0) {
        reward += bananas_collected * 50.0f;  // +50 points for each collected banana
        //std::cout << "Reward for collecting banana: +" << reward << " points" << std::endl;
    }

    //Penalty for object collision (dont know if this works yet.)
    if (player.get_is_colliding_with_platform()) {
        reward -= 0.01f;
        //std::cout << "Penalty for collision applied." << endl;
    }

    // Penalty for Time Exceeded - Should not be too large, because every episode is going to fail like this until the agent learns to perform so it is essentially meaningless
    if (time_exceeded) {
        reward -= 10.0f;  // -10 points for exceeding the maximum time without collecting a banana
        //std::cout << "Life clock expired penalty applied: -10 points (from reward_function.cu)" << std::endl;
    }

    // Reward for Level Completion
    if (level_completed) {
        reward += 1000.0f;  // +1000 points for completing the level
        //std::cout << "Reward for completing the level: +1000 points" << std::endl;
    }

    //Edge penalization logic
    const float edge_threshold = 51.0f; //pixels from the edge, change to 51 if no detection
    const float edge_penalty = 0.05f; //additional 5x the standard collision penalty

    float player_x = player.get_x_position();
    float player_y = player.get_y_position();
    const float player_size = 50.0f;

    if (player_x < (edge_threshold - player_size) || //left edge
        player_x > (1000.0f - edge_threshold) || //right edge
        player_y < (edge_threshold - player_size) || //top edge
        player_y > (800.0f - edge_threshold)) {
            reward -= edge_penalty;
            //std::cout << "Edge proximity detected and penalty applied: " << edge_penalty << " points." << endl;
        }

    //Movement Incentive logic

    if (prev_x_position >= 0.0f && prev_y_position >= 0.0f) {//this is always going to be true
        float movement_x = std::abs(player_x - prev_x_position);
        float movement_y = std::abs(player_y - prev_y_position);

        if (movement_x > 0.0f || movement_y > 0.0f) { //if any movement was done at all,
            const float movement_reward = 0.01f;
            reward += movement_reward;
            //std::cout << "Movement reward applied: + " << movement_reward << " points." << std::endl;
        
        }
    } 
    prev_x_position = player_x;
    prev_y_position = player_y;


    //next reward logic...

    return reward;
}
