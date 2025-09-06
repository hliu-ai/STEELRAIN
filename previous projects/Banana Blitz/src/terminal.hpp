#ifndef TERMINAL_HPP
#define TERMINAL_HPP

#include <SFML/Graphics.hpp>
#include <vector>
#include <functional>
#include <string>
#include <iostream>
#include "utility.hpp"
#include "replay_buffer.hpp"
#include "logger.hpp"
#include "ai/training_loop.hpp"
#include "ai/policy_network.hpp"

struct Section {
    sf::RectangleShape bounds;
    sf::Text label;
    std::function<void()> onClick;  // Optional for interactivity
};

class Terminal {
public:
    Terminal(bool& headless_mode, float& acceleration_factor, Utility& utilityRef, ReplayBuffer& replayBuffer, size_t buffer_capacity, 
            float& total_time_in_seconds_ref, Logger& logger_ref, const std::string& run_id_ref, TrainingLoop& training_loop_ref, PolicyNetwork& policy_network_ref, size_t& global_frame_counter_ref);

    void handleEvents();                // Handle terminal events
    void update();                      // Update terminal data
    void render();                      // Render the terminal
    void drawGrid();

    void updateCumulativeRewardGraph(Logger& logger, const std::string& run_id, size_t global_frame_counter);

    void updateLossGraph(TrainingLoop& training_loop, const std::string& run_id, size_t step);

private:
    sf::RenderWindow window;            // The terminal window
    sf::Font font;                      // Font for text
    sf::Text titleText;                 // Title text
    std::vector<Section> sections;      // Dynamic list of terminal sections

    bool& headlessMode;                // Reference to the main headless_mode variable
    float& accelerationFactor;          //Reference to the main acceleration_factor variable
    float& totalTimeInSeconds;

    bool isEditing = false;             // Whether a field is currently being edited
    int editingSectionIndex = -1; //store the index of the editing section, -1 if none
    std::string currentInput;

    void setupText(sf::Text& text, const std::string& str, sf::Vector2f position, unsigned int size = 24);
    void initializeSections();          // Define all terminal sections
    void handleClick(sf::Vector2i mousePosition); // Handle mouse clicks

    int systemFPSIndex;
    int effectiveFPSIndex;
    int replayBufferIndex;
    size_t bufferCapacity;
    size_t& global_frame_counter;
    int sessionRuntimeIndex;

    Utility& utility; //store a reference to utility
    ReplayBuffer& replayBuffer; //store a reference to replay buffer
    Logger& logger;
    std::string run_id;
    TrainingLoop& training_loop;
    PolicyNetwork& policy_network;

    //For section 14 (episode reward graph)
    sf::Texture cumulativeRewardTexture;
    sf::Sprite cumulativeRewardSprite;
    int cumulativeRewardGraphIndex = -1; //check if needed later
    void loadCumulativeRewardGraph(const std::string& imagePath);

    //For section 15
    sf::Texture lossGraphTexture;
    sf::Sprite lossGraphSprite;
    int lossGraphIndex = -1;
    void loadLossGraph(const std::string& imagePath);

    size_t loss_step_counter = 0;

};

#endif
