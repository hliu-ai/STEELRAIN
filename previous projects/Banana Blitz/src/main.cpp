#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include "player.hpp"
#include "environmental_object.hpp"
#include "collectable.hpp"
#include "resource_manager.hpp"
#include "raycasting.hpp"
#include "utility.hpp"
#include "episode_manager.hpp"
#include "experience_manager.hpp"
#include "replay_buffer.hpp"
#include <torch/torch.h>
#include "ai/input_handler.hpp"
#include "ai/policy_network.hpp"
#include "ai/target_network.hpp"
#include "ai/reward_function.hpp"
#include "ai/training_loop.hpp"
#include "logger.hpp"
#include "core/render_manager.hpp"
#include "core/timing_manager.hpp"
#include <iostream>
#include <vector>
#include <memory>
#include <chrono>
#include <thread>
#include <mutex>
#include <ctime>
#include <cstdlib>
#include <cuda_runtime.h>
#include "terminal.hpp"
using namespace std;
using namespace sf;

int main() {
    std::string model_path = "C:/Dev/2d_game_simple/saved_models/breakthrough_bananas_only.pt";
    size_t buffer_capacity = 100000; // Set the desired capacity for the replay buffer, 100,000
    float gamma = 0.99;
    float learning_rate = 2.5e-4;

    sf::RenderWindow window(sf::VideoMode(1000, 800), "Monkey Banana Game"); //Width, Height in pixels
    window.setPosition(sf::Vector2i(280, 320));
    //window.setFramerateLimit(60); commented out, uncapped fps
    std::srand(static_cast<unsigned int>(std::time(nullptr))); //Seeding the random number generator once, for Epsilon logic
    Utility utility;
    std::string run_id = utility.generate_run_id();
    ResourceManager resource_manager;
    if (!resource_manager.load_textures()) {
        std::cerr << "Failed to load textures. Exiting game." << std::endl;
        return -1; // Exit if texture loading fails
    }
    player_class player(resource_manager);
    InputHandler input_handler;
    PolicyNetwork policy_network;
    policy_network.load_model(model_path);
    TargetNetwork target_network;
    RewardFunction reward_function;
    Logger logger(run_id);
    RenderManager render_manager;
    ExperienceManager experience_manager;
    ReplayBuffer replay_buffer(buffer_capacity);
    TrainingLoop training_loop(gamma, learning_rate, policy_network); // Gamma and learning rate

    
    TimingManager timing_manager;

    


    //Type false if manual control desired for debugging
    input_handler.set_ai_controlled(true);

    size_t global_frame_counter = 0;
    size_t copy_policy_network_to_target_network_interval = 10000; //5000 seems good.
    size_t training_interval = 4; //Run training loop every 4 frames
    size_t save_interval = 10000; //5000 seems good, on low end.
    size_t batch_size = 128;

    using Clock = std::chrono::steady_clock;
    using Duration = std::chrono::duration<float>;
    auto last_frame_time = Clock::now(); //need to initialize here for the first loop to run
    auto start_time = Clock::now();
    float accumulated_time = 0.0f;
    float total_time_in_seconds = 0.0f; //initializing global time counter



    bool headless_mode = false;
    float acceleration_factor = 5.0f; //DO not change from 5, terminal is set to spawn at 5.0
    Terminal terminal(headless_mode, acceleration_factor, utility, replay_buffer, buffer_capacity, total_time_in_seconds, logger, run_id, training_loop, policy_network, global_frame_counter);
    EpisodeManager episode_manager(window, player, resource_manager, utility, logger, run_id, terminal); //Logger now stores episode_reward, cumulative value for post-training report, with episode_manager handling its logic functions.

    training_loop.copy_weights_from_policy_to_target(policy_network, target_network, input_handler, global_frame_counter, copy_policy_network_to_target_network_interval);

// Main game loop. The aim of this design was to be a functional DDQN (Double Deep Q Network) implementation.
    while (window.isOpen()) {

        ++global_frame_counter; //Increment the frame. Not sure if efficient, but more focused on just functionality for now.
        terminal.handleEvents(); //Logic for real time updates to the terminal. Pulls from, but does not push anything to the acting or training loop. 
        episode_manager.handle_events(window, logger, run_id, total_time_in_seconds, training_loop, global_frame_counter); // All this does is enable hitting "esc" to close window and end training. All the parameters are a holdover from when I wanted this function to do more than it currently does.
        float delta_time = timing_manager.update_timing_get_delta_time(last_frame_time, start_time, total_time_in_seconds, utility, acceleration_factor); //All frame timing logic, returns delta_time which is critical in other functions such as for physics and utilities.

        if (global_frame_counter == 1) {
            int default_action = 0;
            input_handler.set_current_action_index(default_action, input_handler.get_current_epsilon());
            input_handler.handle_actions(player, delta_time, episode_manager.get_platforms(), episode_manager.get_bananas(), acceleration_factor);
            std::cout << "Priming environment on frame 1 with default action." << std::endl;
            continue; // Skip recording experience for frame 1.
        }

        const auto& raycasting = player.get_raycasting();
        const auto& proximity_radius = player.get_proximity_radius();

        // Create the state tensor (s)
        torch::Tensor state_tensor = input_handler.get_combined_inputs(raycasting, proximity_radius, player);


        // Create the action tensor (a)
        torch::Tensor output_q_tensor = policy_network.forward(state_tensor);
        torch::Tensor max_q_value, max_q_index;
        std::tie(max_q_value, max_q_index) = output_q_tensor.max(1); // Find the action with the highest Q-value
        torch::Tensor action_tensor = max_q_index.unsqueeze(1); // Shape [1, 1]
        int action_index = action_tensor.item<int>(); // Get the action index as an integer
        
        // Update the current action index in the input handler and apply the action to the environment
        input_handler.set_current_action_index(action_index, input_handler.get_current_epsilon());
        input_handler.handle_actions(player, delta_time, episode_manager.get_platforms(), episode_manager.get_bananas(), acceleration_factor); //In human mode, arrow key inputs lead to player positional updates. In AI mode (the most important one of course) the action index (0-5) which comes from the training loop is mapped to up, down, left, right, and idle. I have some holdover functions in input_handler from when I wanted to implement epsilon, but I didn't fully understand how it would work in my environment so currently I am not doing anything with epislon.
       
        
        
        //Update additional elements
        utility.update(window, delta_time, input_handler, player.get_is_colliding_with_platform(), acceleration_factor); //Updates a number of on-screen indicators such as the life clock, collision indicator, dpad showing which action the player is taking. Not related to training, just a precursor to our new terminal window.
        int bananas_collected = episode_manager.collect_bananas_and_update_life_clock(player, utility); //Increases the amount of life time remaining in the episode for the player when a banana is collected
        bool level_completed = false, time_exceeded = false;

        //Gets the d in the experience tuple (s, a, r, s', d)
        bool done = episode_manager.get_and_handle_done(level_completed, time_exceeded, logger, utility); //Checks if win OR loss condition is met and handles the episode end logic if met. Updates "done" variable, which will be the d in (s, a, r, s', d).
        // Gets the r in (s, a, r, s', d) and set reward for logger
        float reward = episode_manager.calculate_rewards(reward_function, player, bananas_collected, delta_time, level_completed, time_exceeded, logger, acceleration_factor); //Gets the reward which is calculated from  a function in reward_function.cu and makes sure logger gets it as well for graphing. I expect to do a lot of tweaking regarding the reward function, as it seems like a component with the highest degree of modifiability and impact once we ensure we have this design right.
       
        //Now capture (s') AFTER the action has been applied
        const auto& next_raycasting = player.get_raycasting();
        const auto& next_proximity_radius = player.get_proximity_radius();
        torch::Tensor next_state_tensor = input_handler.get_combined_inputs(next_raycasting, next_proximity_radius, player); 


        //Experience creation begins below



        //Big refactoring done: compressed generation of all 5 experience tensors into one line. First significant implementation of DDQN
        auto experience = experience_manager.create_experience(player, input_handler, policy_network, reward, done, state_tensor, action_tensor, next_state_tensor);

        replay_buffer.add_experience(experience);

        //Execute whole training step every training_interval. Big line of code here
        training_loop.train_if_ready(global_frame_counter, training_interval, replay_buffer, batch_size, target_network);

        //Performed every copy_interval frames
        training_loop.copy_weights_from_policy_to_target(policy_network, target_network, input_handler, global_frame_counter, copy_policy_network_to_target_network_interval);

        // Save the model every save_interval frames
        if (global_frame_counter % save_interval == 0) {
            training_loop.save_model(policy_network, "C:/Dev/2d_game_simple/saved_models", run_id, global_frame_counter);
        }

        if (headless_mode) {
            window.setVisible(false);
        } else {
            window.setVisible(true);
        }

        if (!headless_mode) {
            render_manager.draw_all(window, player, utility, episode_manager);
            window.display();
        }

        terminal.update();
        terminal.render();
    }
    return 0;
}

//go to episode manager to get terminal outputs again