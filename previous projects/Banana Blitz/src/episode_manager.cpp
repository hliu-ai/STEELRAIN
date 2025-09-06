// EpisodeManager.cpp
#include "episode_manager.hpp"
#include "ai/reward_function.hpp"
#include <iostream>
#include "logger.hpp"
#include "utility.hpp"
#include "player.hpp"
#include "terminal.hpp"


EpisodeManager::EpisodeManager(sf::RenderWindow &window, player_class &player, ResourceManager &resource_manager, Utility &utility, Logger &logger_ref, const std::string& run_id_ref, Terminal& terminal_ref)
    : window(window), player(player), resource_manager(resource_manager), utility(utility), logger(logger_ref), run_id(run_id_ref), terminal(terminal_ref){
    reset_episode(logger, terminal, run_id); //One reset occurs at game start just to ensure consistent starting environment to environment after resets
}

void EpisodeManager::reset_episode(Logger &logger, Terminal& terminal, const std::string& run_id) {
    // Reset player position
    player.reset_position();

    // Re-create platforms
    platforms.clear();
    auto platform_positions = environmental_object::get_x_platform_positions(0);
    for (const auto &position : platform_positions) {
        float x, y, width, height;
        sf::Color color;
        std::tie(x, y, width, height, color) = position;
        platforms.emplace_back(x, y, width, height, color);
    }

    // Re-create collectables (bananas)
    bananas.clear();
    bananas = collectable::get_mega_level_bananas(resource_manager); //BANANAS

    // Reset banana counters
    bananas_remaining = bananas.size();
    utility.resetTimer();
    utility.resetLifeClock(5.0f); // Reset life clock to initial value


    //std::cout << "Episode reward to log: " << logger.get_episode_reward() << std::endl;
    logger.logEpisodeReward(logger.get_episode_reward()); //only need to log the episode reward once upon reset: - not in the main loop.
    logger.reset_episode_reward();

    //Update the graph in the terminal
    //terminal.updateCumulativeRewardGraph(logger, run_id);
}




void EpisodeManager::check_win_or_lose_conditions(bool &level_completed, bool &time_exceeded, Utility &utility) {
    // Check if all bananas are collected (win condition)
    if (bananas_remaining == 0) {
        level_completed = true;
    }
    // Check if player hasn't collected a banana within the max allowed time (lose condition)
    if (utility.getLifeClock() <= 0.0f) {
        time_exceeded = true;
    }
}

float EpisodeManager::calculate_rewards(RewardFunction &reward_function, player_class &player, int bananas_collected, float delta_time, bool level_completed, bool time_exceeded, Logger& logger, float acceleration_factor) {
    float reward = reward_function.calculate_reward(player, bananas_collected, delta_time, level_completed, time_exceeded, acceleration_factor);
    logger.set_episode_reward(reward);
    return reward;
}


void EpisodeManager::handle_episode_end(bool level_completed, Logger &logger) {
    if (level_completed) {
        //std::cout << "All bananas collected! Resetting game. Total episode_reward from this episode: " << logger.get_episode_reward() << " points." << std::endl;
    } else {
        //std::cout << "Too much time without collecting a banana! Resetting game. Total episode_reward from this episode: " << logger.get_episode_reward() << " points." << std::endl;
    }
    reset_episode(logger, terminal, run_id);
}


void EpisodeManager::handle_events(sf::RenderWindow &window, Logger &logger, const std::string& run_id, float total_time_in_seconds, TrainingLoop& training_loop, size_t global_frame_counter) {
    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed || sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)) {
           
           /*
            // Save and plot reward graph
            std::string reward_graph_path = "C:/Dev/2d_game_simple/saved_logs/reward_plot_" + std::to_string(global_frame_counter) + ".png";
            logger.saveAndPlotRewards(run_id, reward_graph_path);

           
           
            // Save and plot loss graph
            std::string loss_data_path = "C:/Dev/2d_game_simple/saved_logs/loss_data_" + run_id + ".csv";
            training_loop.save_loss_to_csv(loss_data_path, global_frame_counter);
            std::string loss_graph_path = "C:/Dev/2d_game_simple/saved_logs/loss_plot_" + std::to_string(global_frame_counter) + ".png";
            std::string command = "\"C:/Users/hanse/AppData/Local/Programs/Python/Python312/python.exe\" "
                                  "C:/Dev/2d_game_simple/python_logging/generate_plot.py loss "
                                  + loss_data_path + " " + loss_graph_path;
            system(command.c_str());
            */
            // Close window
            window.close();
        }
    }
}



int EpisodeManager::handle_banana_collection(player_class &player) {
    int bananas_collected = 0;
    int initial_bananas_remaining = bananas.size();
    for (auto it = bananas.begin(); it != bananas.end();) {
        if ((*it)->is_collected(player.get_player_sprite())) {
            //std::cout << "Banana collected!" << std::endl;
            bananas_collected++;
            it = bananas.erase(it);
            if (bananas.size() < initial_bananas_remaining) {
                if (!bananas.empty()) {
                    //(*bananas.begin())->collect_sound.play();
                }
            } // Remove the collected banana from the vector
            bananas_remaining = bananas.size(); // Update the count of remaining bananas
        } else {
            ++it;
        }
    }
    return bananas_collected;
}

void EpisodeManager::draw_all(sf::RenderWindow &window) {
    // Draw platforms
    for (const auto &platform : platforms) {
        platform.draw(window);
    }

    // Draw collectables
    for (const auto &banana : bananas) {
        window.draw(banana->get_collectable_sprite());
    }

    // Draw player
    window.draw(player.get_player_sprite());
}

std::vector<environmental_object>& EpisodeManager::get_platforms() {
    return platforms;
}

std::vector<std::unique_ptr<collectable>>& EpisodeManager::get_bananas() {
    return bananas;
}

int EpisodeManager::get_bananas_remaining() const {
    return bananas_remaining;
}


int EpisodeManager::collect_bananas_and_update_life_clock(player_class& player, Utility& utility) {
    // Handle banana collection
    int bananas_collected = handle_banana_collection(player);

    // Increase the life clock based on the number of bananas collected
    if (bananas_collected > 0) {
        utility.increaseLifeClock(2.0f * bananas_collected);
    }
    //Return the number of bananas collected
    return bananas_collected;
}

bool EpisodeManager::get_and_handle_done(bool& level_completed, bool& time_exceeded, Logger& logger, Utility& utility) {
    // Check win or lose conditions
    check_win_or_lose_conditions(level_completed, time_exceeded, utility);

    // Handle episode end if necessary
    if (level_completed || time_exceeded) {
        handle_episode_end(level_completed, logger);
        return true; // Done
    }

    return false; // Not done
}