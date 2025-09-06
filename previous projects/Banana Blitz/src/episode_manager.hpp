// EpisodeManager.hpp
#ifndef EPISODE_MANAGER_HPP
#define EPISODE_MANAGER_HPP

#include <SFML/Graphics.hpp>
#include "player.hpp"
#include "environmental_object.hpp"
#include "collectable.hpp"
#include "resource_manager.hpp"
#include "utility.hpp"
#include "ai/reward_function.hpp"
#include <vector>
#include <memory>
#include "logger.hpp"
#include "terminal.hpp"
#include "ai/training_loop.hpp"

class EpisodeManager {
public:
    EpisodeManager(sf::RenderWindow &window, player_class &player, ResourceManager &resource_manager, Utility &utility, Logger &logger_ref, const std::string& run_id_ref, Terminal& terminal_ref);

    // Episode management
    void reset_episode(Logger &logger, Terminal &terminal, const std::string& run_id);

    void handle_events(sf::RenderWindow &window, Logger &logger, const std::string& run_id, 
                   float total_time_in_seconds, TrainingLoop& training_loop, size_t global_frame_counter);

    int handle_banana_collection(player_class &player);

    void draw_all(sf::RenderWindow &window);
    void check_win_or_lose_conditions(bool &level_completed, bool &time_exceeded, Utility &utility);
    float calculate_rewards(RewardFunction &reward_function, player_class &player, int bananas_collected, float delta_time, bool level_completed, bool time_exceeded, Logger& logger, float acceleration_factor);
    void handle_episode_end(bool level_completed, Logger &logger); //doesnt need time_exceeded, handled in the logic.


    int collect_bananas_and_update_life_clock(player_class& player, Utility& utility);

    bool get_and_handle_done(bool& level_completed, bool& time_exceeded, Logger& logger, Utility& utility);


    // Accessors
    std::vector<environmental_object>& get_platforms();
    std::vector<std::unique_ptr<collectable>>& get_bananas();
    int get_bananas_remaining() const;

private:
    sf::RenderWindow &window;
    player_class &player;
    ResourceManager &resource_manager;
    Utility &utility;
    Terminal &terminal;
    Logger &logger;
    std::string run_id;

    std::vector<environmental_object> platforms;
    std::vector<std::unique_ptr<collectable>> bananas;
    int bananas_remaining;
};

#endif // EPISODE_MANAGER_HPP