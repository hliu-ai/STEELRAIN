#include "Logger.hpp"
#include <fstream>
#include <filesystem>
#include <iostream>
#include <cstdlib>
#include <numeric>

// Constructor
Logger::Logger(const std::string& run_id) {
    // File paths for reward and loss CSV files
    reward_filename = "C:/Dev/2d_game_simple/saved_logs/episode_rewards_" + run_id + ".csv";
    loss_filename = "C:/Dev/2d_game_simple/saved_logs/loss_values_" + run_id + ".csv";
}

// Log an episode reward
void Logger::logEpisodeReward(float episode_reward) {
    episode_rewards.push_back(episode_reward);
}

void Logger::saveAndPlotRewards(const std::string& run_id, const std::string& output_image_path) {
    // Save rewards to CSV
    std::ofstream file(reward_filename);
    if (file.is_open()) {
        file << "Episode,Reward\n";
        for (size_t i = 1; i < episode_rewards.size(); ++i) {
            file << (i - 1) << "," << episode_rewards[i] << "\n";
        }
        file.close();
    }

    // Generate reward plot using Python with custom output image path
    std::string command = "\"C:/Users/hanse/AppData/Local/Programs/Python/Python312/python.exe\" "
                          "C:/Dev/2d_game_simple/python_logging/generate_plot.py reward " 
                          + reward_filename + " " + output_image_path;
    system(command.c_str());
}


// Save loss data and plot the loss graph 
void Logger::saveAndPlotLoss(const std::string& run_id, const std::vector<float>& loss_values, size_t step) {
    // Save loss data to CSV
    std::ofstream file(loss_filename, std::ios::app); // Append mode
    if (file.is_open()) {
        for (size_t i = 0; i < loss_values.size(); ++i) {
            file << (step + i) << "," << loss_values[i] << "\n";
        }
        file.close();
    } else {
        std::cerr << "Error: Could not open loss CSV file for writing.\n";
        return;
    }

    // Generate loss plot using Python
    std::string output_image = "C:/Dev/2d_game_simple/saved_logs/loss_plot_" + run_id + ".png";
    std::string command = "\"C:/Users/hanse/AppData/Local/Programs/Python/Python312/python.exe\" "
                          "C:/Dev/2d_game_simple/python_logging/generate_plot.py loss " + loss_filename + " " + output_image;
    system(command.c_str());
}

// Getter for episode reward
float Logger::get_episode_reward() {
    return episode_reward;
}

// Setter for episode reward
void Logger::set_episode_reward(float reward) {
    episode_reward += reward;
}

// Reset episode reward
void Logger::reset_episode_reward() {
    episode_reward = 0.0f;
}
