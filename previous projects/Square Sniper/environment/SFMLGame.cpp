#include "SFMLGame.hpp"
#include <iostream>
#include <algorithm>
#include <cmath>

SFMLGame::SFMLGame()
    : window(sf::VideoMode(500, 500), "SFML Target Shooting"),
    target(),
    hud(),
    crosshair(),
    crosshairPos(250.f, 250.f),
    mousePreviouslyPressed(false),
    cumulative_reward(0),//score is our cumulative reward
    agentMode(true), // Default to agent mode; can be changed later
    scaleFactor(10.f)
{
    window.setFramerateLimit(60);
}

SFMLGame::~SFMLGame() {
    // Destructor can perform cleanup if needed.
}

std::vector<float> SFMLGame::reset() {
    // Reset game variables.
    cumulative_reward = 0;
    hud.reset();
    target.resetPosition();
    crosshairPos = sf::Vector2f(250.f, 250.f);
    // Clear any pending window events.
    sf::Event event;
    while (window.pollEvent(event)) {}

    //Initialize the episode timer
	episode_start_time = std::chrono::steady_clock::now();
    prev_time = episode_start_time;

    // Return the initial state.
    return generateStateArray(target.getShape(), crosshairPos);
}

// --- STEP ---

std::tuple<std::vector<float>, float, bool> SFMLGame::step(const Action& action) {
    reward = 0;
    done = false;

    float hit_reward = 100.0;
    float shaping_r_max = 0.5f;
    float shaping_k = 0.02;
    float penalty_per_second = 1.0;
    float shooting_penalty = 1.0;
    

    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed)
            window.close();
    }

    //REWARD - TIME PENALTY
	auto current_time = std::chrono::steady_clock::now();
    std::chrono::duration<float> delta_time = current_time - prev_time;
    prev_time = current_time;
    time_penalty = static_cast<float>(delta_time.count()) * penalty_per_second; //10 point per second penalty
    reward -= time_penalty;
    //std::cout << "Time penalty applied: " << time_penalty << "\n";

    if (agentMode) {
        // AGENT MODE: update crosshair based on action.
        crosshairPos.x += action.delta_x * scaleFactor;
        crosshairPos.y += action.delta_y * scaleFactor;
        crosshairPos.x = std::clamp(crosshairPos.x, 0.f, 500.f);
        crosshairPos.y = std::clamp(crosshairPos.y, 0.f, 500.f);

        //REWARD - SHAPING
        // Compute the target's center
        sf::FloatRect targetBounds = target.getShape().getGlobalBounds();
        sf::Vector2f targetCenter(targetBounds.left + targetBounds.width / 2, targetBounds.top + targetBounds.height / 2);
        //Compute Euclidean distance between crosshair and target center
        float dx = crosshairPos.x - targetCenter.x;
        float dy = crosshairPos.y - targetCenter.y;
        float distance_to_center = std::sqrt(dx * dx + dy * dy);
        // std::cout << "distance_to_center: " << distance_to_center << '\n';//debugging output - confirmed functionality!
        // Apply the exponential shaping reward
        float shapingReward = shaping_r_max * std::exp(-shaping_k * distance_to_center);
        reward += shapingReward;


        //REWARD - SHOOTING PENALTY
        // Process shooting action.
        if (action.shoot == 1 && hud.shotsRemaining() > 0) {
            hud.decrementShots();
            reward -= shooting_penalty; //Penalty for shooting
            //std::cout << "Penalty for shooting applied: -1" << "\n";
            if (target.getShape().getGlobalBounds().contains(crosshairPos)) {
                reward += hit_reward; //points for successful hit
                std::cout << "Reward for a successful hit applied: +" << hit_reward << "\n";
                target.resetPosition();
            }
            else {
                //std::cout << "Miss! Score: " << cumulative_reward << "\n";
            }
        }
    }







    //HUMAN MODE - do not touch, no longer in active use
    else {
        // HUMAN MODE: use mouse input.
        crosshairPos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
        crosshairPos.x = std::clamp(crosshairPos.x, 0.f, 500.f);
        crosshairPos.y = std::clamp(crosshairPos.y, 0.f, 500.f);
        bool mouseCurrentlyPressed = sf::Mouse::isButtonPressed(sf::Mouse::Left);

        //REWARD - SHAPING
        // Compute the target's center
        sf::FloatRect targetBounds = target.getShape().getGlobalBounds();
        sf::Vector2f targetCenter(targetBounds.left + targetBounds.width / 2, targetBounds.top + targetBounds.height / 2);
        //Compute Euclidean distance between crosshair and target center
        float dx = crosshairPos.x - targetCenter.x;
        float dy = crosshairPos.y - targetCenter.y;
        float distance_to_center = std::sqrt(dx * dx + dy * dy);
        std::cout << "distance_to_center: " << distance_to_center << '\n';//debugging output
        // Apply the exponential shaping reward
        float shapingReward = shaping_r_max * std::exp(-shaping_k * distance_to_center);
        reward += shapingReward;
        if (!mousePreviouslyPressed && mouseCurrentlyPressed && hud.shotsRemaining() > 0) {
            hud.decrementShots();
            reward -= shooting_penalty; //Penalty for shooting
            if (target.getShape().getGlobalBounds().contains(crosshairPos)) {
                reward += hit_reward;
                target.resetPosition();
                std::cout << "Reward for a successful hit +50 human mode output only " << "\n";
            }
            else {
                //std::cout << "Miss! Score: " << cumulative_reward << "\n";
            }
        }
        mousePreviouslyPressed = mouseCurrentlyPressed;
    }
    // End of Human mode


    cumulative_reward += reward; //update the cumulative reward

    // Rendering: clear, draw objects, and display.
    window.clear(sf::Color::Black);
    window.draw(target.getShape());
    crosshair.updatePosition(crosshairPos);
    crosshair.draw(window);
    hud.update(window);
    hud.draw(window);
    window.display();

    // Episode reset check: if the HUD indicates the episode is over,
    // reset score, HUD, target, and crosshair, and mark done.
    if (hud.isEpisodeOver()) {
        done = true;
        //std::cout << "Episode ended. Cumulative Reward: " << cumulative_reward << "\n";
        hud.reset();
        target.resetPosition();
        crosshairPos = sf::Vector2f(250.f, 250.f);
    }

    // Retrieve the current state.
    std::vector<float> state = generateStateArray(target.getShape(), crosshairPos);

    
    //std::cout << "Done: " << done << "\n";
    return std::make_tuple(state, reward, done);
}


std::vector<float> SFMLGame::get_state() const {
    return generateStateArray(target.getShape(), crosshairPos);
}

void SFMLGame::set_agent_mode(bool mode) {
    agentMode = mode;
}

bool SFMLGame::is_open() const {
    return window.isOpen();
}
