#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <string>
#include <vector>

class Logger {
public:
    // Constructor
    Logger(const std::string& run_id);

    // Reward-related methods
    void logEpisodeReward(float episode_reward);
    void saveAndPlotRewards(const std::string& run_id, const std::string& output_image_path);


    // Loss-related methods
    void saveAndPlotLoss(const std::string& run_id, const std::vector<float>& loss_values, size_t step);

    // Getters and setters for rewards
    float get_episode_reward();
    void set_episode_reward(float reward);
    void reset_episode_reward();

private:
    std::string reward_filename;
    std::string loss_filename;

    std::vector<float> episode_rewards;
    float episode_reward = 0.0f;
};

#endif
