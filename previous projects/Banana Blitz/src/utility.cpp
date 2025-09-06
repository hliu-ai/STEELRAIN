#include "utility.hpp"
#include <SFML/Graphics.hpp>
#include "ai/input_handler.hpp"
#include <iostream> // Include iostream for cerr
#include <sstream>
#include <iomanip>
#include <chrono>
#include <ctime>

using namespace std;
using namespace sf;

// Constructor for Utility
Utility::Utility(float initial_life_clock, float max_episode_time)
    : life_clock(initial_life_clock), 
      max_time_per_episode(max_episode_time), 
      elapsed_time(0.0f) {// Initialize elapsed_time 


    // Setup episode timer text and life clock text
    if (!font.loadFromFile("C:/Dev/2d_game_simple/src/data/fonts/Lato-Regular.ttf")) {
        std::cerr << "Failed to load font." << std::endl;
    }
    timer_text.setFont(font);
    timer_text.setCharacterSize(20);
    timer_text.setFillColor(Color::Black);
    timer_text.setPosition(10, 10);

    life_clock_text.setFont(font);
    life_clock_text.setCharacterSize(20);
    life_clock_text.setFillColor(Color::Black);
    life_clock_text.setPosition(10, 40);

    // Define squares for directional feedback as member variables
    up_square.setSize(Vector2f(30.0f, 50.0f));
    down_square.setSize(Vector2f(30.0f, 50.0f));
    left_square.setSize(Vector2f(50.0f, 30.0f));
    right_square.setSize(Vector2f(50.0f, 30.0f));

    float base_x = 100.0f; // Adjust position to move down and to the right
    float base_y = 150.0f;

    // Setup the "idle" square in the center
    idle_square.setSize(Vector2f(30.0f, 30.0f));
    idle_square.setOrigin(15.0f, 15.0f);
    idle_square.setPosition(base_x, base_y);
    idle_square.setOutlineThickness(2);
    idle_square.setOutlineColor(Color(0, 0, 0, 100));
    idle_square.setFillColor(Color(0, 255, 0, 100)); // Transparent green to indicate idle

    // Setup the squares for up, down, left, right
    up_square.setOrigin(15.0f, 25.0f);
    up_square.setPosition(base_x, base_y - 45.0f);
    up_square.setOutlineColor(Color(0, 0, 0, 100));
    up_square.setOutlineThickness(2);
    up_square.setFillColor(Color(0, 255, 0, 50));

    down_square.setOrigin(15.0f, 25.0f);
    down_square.setPosition(base_x, base_y + 45.0f);
    down_square.setOutlineColor(Color(0, 0, 0, 100));
    down_square.setOutlineThickness(2);
    down_square.setFillColor(Color(0, 255, 0, 50));

    left_square.setOrigin(25.0f, 15.0f);
    left_square.setPosition(base_x - 45.0f, base_y);
    left_square.setOutlineColor(Color(0, 0, 0, 100));
    left_square.setOutlineThickness(2);
    left_square.setFillColor(Color(0, 255, 0, 50));

    right_square.setOrigin(25.0f, 15.0f);
    right_square.setPosition(base_x + 45.0f, base_y);
    right_square.setOutlineColor(Color(0, 0, 0, 100));
    right_square.setOutlineThickness(2);
    right_square.setFillColor(Color(0, 255, 0, 50));


    fps_text.setFont(font);
    fps_text.setCharacterSize(20);
    fps_text.setFillColor(sf::Color(0, 100, 0));
    fps_text.setPosition(700, 10); // Position below other text

    collision_indicator.setSize(Vector2f(130.0f, 20.0f));  // Thin, wide rectangle
    collision_indicator.setPosition(base_x - 65.0f, base_y + 80.0f);  // Position just below the D-pad
    collision_indicator.setOutlineThickness(2);
    collision_indicator.setOutlineColor(Color(0, 0, 0, 100));  // Semi-transparent black outline
    collision_indicator.setFillColor(Color::Transparent);  // Transparent fill initially

        // In the Utility constructor, initialize `collision_feedback_text`
    collision_feedback_text.setFont(font);
    collision_feedback_text.setCharacterSize(15);  // Small font size
    collision_feedback_text.setFillColor(Color::Black);
    collision_feedback_text.setString("Red during active collision.");
    collision_feedback_text.setPosition(collision_indicator.getPosition().x - 20, collision_indicator.getPosition().y + 25.0f);  // Position just below the collision feedback rectangle


}

void Utility::update(RenderWindow &window, float delta_time, const InputHandler& input_handler, bool is_colliding_with_platform, float acceleration_factor) {
    int ai_action = input_handler.get_current_action_index(); //Only used for utility dpad. Not epsilon
    
    float adjusted_delta_time = delta_time * acceleration_factor;
    elapsed_time += adjusted_delta_time;

    // Update the timer text based on the new delta_time
    timer_text.setString("Episode Time Elapsed: " + std::to_string(static_cast<int>(elapsed_time)) + "." +
                         std::to_string(static_cast<int>((elapsed_time - static_cast<int>(elapsed_time)) * 100)) +
                         "s | Episode will hard reset at " + std::to_string(static_cast<int>(max_time_per_episode)) + "s");

    // Update the life clock text based on remaining life
    life_clock_text.setString("Life Clock Remaining: " + std::to_string(static_cast<int>(life_clock)) + "." +
                              std::to_string(static_cast<int>((life_clock - static_cast<int>(life_clock)) * 100)) + "s");

    // Map AI action to directional input visuals
    bool up = (ai_action == 1);
    bool down = (ai_action == 2);
    bool left = (ai_action == 3);
    bool right = (ai_action == 4);

    setActionFeedback(up, down, left, right, is_colliding_with_platform);

    // Debug output for collision status
    //std::cout << "Collision status: " << (is_colliding_with_platform ? "Colliding" : "Not colliding") << std::endl;

    // Set color based on collision status
    if (is_colliding_with_platform) {
        collision_indicator.setFillColor(Color(255, 0, 0, 100));  // Semi-transparent red on collision
    } else {
        collision_indicator.setFillColor(Color::Transparent);  // Transparent when no collision
    }
}


void Utility::update_fps_counter(float delta_time, float acceleration_factor) {
    frame_count++;

    float adjusted_delta_time = delta_time * acceleration_factor;
    //Accumulate real (unscaled) delta_time WALLCLOCK time
    accumulated_time += adjusted_delta_time;

    //Calculate FPS once every second of real time
    if (accumulated_time >= 1.0f) {
        //Real FPS: unscaled by acceleration factor
        float real_fps = frame_count / accumulated_time;

        //Effective FPS: Scaled by acceleration factor - Represents how fast the environment progresses in effective game time
        float effective_fps = real_fps * acceleration_factor;

        //Update the fps text display
         fps_text.setString("Real FPS: " + std::to_string(static_cast<int>(real_fps)) +
                           "\nEffective FPS: " + std::to_string(static_cast<int>(effective_fps)));
        //std::cout << "Real FPS: " << static_cast<int>(real_fps)
         //         << ", Effective FPS: " << static_cast<int>(effective_fps) << std::endl;

        
        current_system_FPS = real_fps;
        current_effective_FPS = effective_fps;

        // Reset for the next calculation
        frame_count = 0;
        accumulated_time = 0.0f;
    }
}

void Utility::resetTimer() {
    elapsed_time = 0.0f;
}

void Utility::resetLifeClock(float initial_life_clock) {
    life_clock = initial_life_clock;
}

void Utility::resetBananaTimer() {
    life_clock = 5.0f; // Reset to initial value or use parameter
}

void Utility::resetEpisode() {
    elapsed_time = 0.0f;
    life_clock = 5.0f; // Reset to initial life clock or set dynamically
}

void Utility::updateTimers(float delta_time, bool banana_collected, float acceleration_factor) {
    // Update elapsed time
    float adjusted_delta_time = delta_time * acceleration_factor;
    elapsed_time += adjusted_delta_time;
    timer_text.setString("Episode Time Elapsed: " + std::to_string(static_cast<int>(elapsed_time)) + "." +
                         std::to_string(static_cast<int>((elapsed_time - static_cast<int>(elapsed_time)) * 100)) + "s");

    // Update life clock based on delta_time and collection events
    if (banana_collected) {
        life_clock += 2.0f; // Reward 2 extra seconds of life per collected banana
        if (life_clock > max_time_per_episode) {
            life_clock = max_time_per_episode; // Cap life clock at max_time_per_episode
        }
    } else {
        life_clock -= adjusted_delta_time;
        if (life_clock < 0.0f) {
            life_clock = 0.0f; // Prevent life_clock from becoming negative
        }
    }
    life_clock_text.setString("Life Clock Remaining: " + std::to_string(static_cast<int>(life_clock)) + "." +
                              std::to_string(static_cast<int>((life_clock - static_cast<int>(life_clock)) * 100)) + "s");
}

void Utility::updateLifeClock(float delta_time, float acceleration_factor) {
    // Update the life clock by decreasing it with elapsed time
    float adjusted_delta_time = delta_time * acceleration_factor;
    life_clock -= adjusted_delta_time;
    if (life_clock < 0.0f) {
        life_clock = 0.0f; // Prevent life_clock from becoming negative
    }
}


void Utility::increaseLifeClock(float increment) {
    life_clock += increment;
    if (life_clock > max_time_per_episode) {
        life_clock = max_time_per_episode; // Cap life clock at maximum time allowed per episode
    }
}


void Utility::setActionFeedback(bool up, bool down, bool left, bool right, bool is_colliding_with_platform) { //currently not using is_colliding with platform. debugging
    // Reset all squares to transparent
    up_square.setFillColor(Color::Transparent);
    down_square.setFillColor(Color::Transparent);
    left_square.setFillColor(Color::Transparent);
    right_square.setFillColor(Color::Transparent);
    idle_square.setFillColor(Color(0, 255, 0, 100)); // Green to indicate idle

    //Color outline_color = is_colliding_with_platform ? Color::Red : Color(0, 0, 0, 100);  // Red if colliding, default otherwise THIS IS WHERE WE PUT THE COLLISION SHIELD VISUAL

    // Update visual feedback based on provided actions
    if (up) {
        up_square.setFillColor(Color(0, 255, 0, 100));
        idle_square.setFillColor(Color::Transparent);
    }
    if (down) {
        down_square.setFillColor(Color(0, 255, 0, 100));
        idle_square.setFillColor(Color::Transparent);
    }
    if (left) {
        left_square.setFillColor(Color(0, 255, 0, 100));
        idle_square.setFillColor(Color::Transparent);
    }
    if (right) {
        right_square.setFillColor(Color(0, 255, 0, 100));
        idle_square.setFillColor(Color::Transparent);
    }
}

void Utility::draw(RenderWindow &window) {
    window.draw(idle_square);
    window.draw(up_square);
    window.draw(down_square);
    window.draw(left_square);
    window.draw(right_square);
    window.draw(timer_text);
    window.draw(life_clock_text);
    window.draw(fps_text); // Draw FPS counter
    window.draw(collision_indicator);
    window.draw(collision_feedback_text);
}

float Utility::getLifeClock() const {
    return life_clock;
}


// Function to generate a timestamp-based run ID
std::string Utility::generate_run_id() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm now_tm = *std::localtime(&now_time_t);

    std::ostringstream oss;
    oss << std::put_time(&now_tm, "%Y%m%d_%H%M%S");
    return oss.str();
}

