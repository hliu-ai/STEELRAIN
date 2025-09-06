#ifndef UTILITY_HPP
#define UTILITY_HPP

#include <SFML/Graphics.hpp>
#include "ai/input_handler.hpp"
#include <string>
#include <chrono>

class Utility {
public:
    Utility(float initial_life_clock = 5.0f, float initial_max_time_per_episode = 60.0f);
    // Reset functions
    void resetTimer();
    void resetLifeClock(float initial_life_clock);
    void resetBananaTimer(); // Reset life clock after collecting a banana
    void resetEpisode(); // Resets life clock and elapsed time at the start of a new episode

    // Update functions
    void update(sf::RenderWindow &window, float delta_time, const InputHandler& input_handler, bool is_colliding_with_platform, float acceleration_factor);

    void updateLifeClock(float delta_time, float acceleration_factor); // Update life clock by decrementing it based on time elapsed
    void updateTimers(float delta_time, bool banana_collected, float acceleration_factor); // Update both the elapsed time and life clock
    void increaseLifeClock(float increment); // Increase the life clock (used after collecting bananas)

    // Draw functions
    void draw(sf::RenderWindow &window);

    // Getter functions
    float getLifeClock() const;


    void update_fps_counter(float delta_time, float acceleration_factor);

    // Function to generate a timestamp-based run ID to be used with saving
    std::string generate_run_id();

    //For terminal
    float get_system_FPS() const { return current_system_FPS; }
    float get_effective_FPS() const { return current_effective_FPS; }

private:
    // Function to set action feedback based on key presses
    void setActionFeedback(bool up, bool down, bool left, bool right, bool is_colliding_with_platform);

    // Member variables
    sf::RectangleShape up_square;
    sf::RectangleShape down_square;
    sf::RectangleShape left_square;
    sf::RectangleShape right_square;
    sf::RectangleShape idle_square;
    sf::Font font;
    sf::Text timer_text;
    sf::Text life_clock_text;

    float life_clock; // Life clock variable
    float max_time_per_episode; // Maximum time allowed per episode
    float elapsed_time; // Episode elapsed time


    sf::Text fps_text; // Text for displaying FPS
    int frame_count = 0;
    float accumulated_time = 0.0f;
    std::chrono::steady_clock last_frame_time;


    sf::RectangleShape collision_indicator; // New rectangle for collision feedback
    sf::Text collision_feedback_text;  // Text for the collision indicator explanation

    float initial_life_clock;
    float max_episode_time;

    float current_system_FPS = 0.0f;
    float current_effective_FPS = 0.0f;

};

#endif // UTILITY_HPP
