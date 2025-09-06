#ifndef REWARD_FUNCTION_HPP
#define REWARD_FUNCTION_HPP

#include "player.hpp"
#include "collectable.hpp"
#include <vector>
#include <memory>

class RewardFunction {
public:
    // Constructor
    RewardFunction();

    // Calculate rewards based on the current state of the game environment
    float calculate_reward(const player_class &player, int bananas_collected, float delta_time, bool level_completed, bool time_exceeded, float acceleration_factor);

private:
    float prev_x_position;
    float prev_y_position;
};

#endif // REWARD_FUNCTION_HPP
