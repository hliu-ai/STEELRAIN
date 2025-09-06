#include "terminal.hpp"
#include <iostream>

// Constructor
Terminal::Terminal(bool& headless_mode, float& acceleration_factor, Utility& utilityRef, ReplayBuffer& replay_buffer, size_t buffer_capacity, 
                    float& total_time_in_seconds_ref, Logger& logger_ref, const std::string& run_id_ref, TrainingLoop& training_loop_ref, PolicyNetwork& policy_network_ref, size_t& global_frame_counter_ref) 
    : headlessMode(headless_mode)
    , accelerationFactor(acceleration_factor)
    , utility(utilityRef)
    , replayBuffer(replay_buffer)
    , bufferCapacity(buffer_capacity)
    , totalTimeInSeconds(total_time_in_seconds_ref)
    , logger(logger_ref)
    , run_id(run_id_ref)
    , training_loop(training_loop_ref)
    , policy_network(policy_network_ref)
    , global_frame_counter(global_frame_counter_ref)
    {
    // Create terminal window
    window.create(sf::VideoMode(1000, 800), "Terminal");
    window.setPosition(sf::Vector2i(1280, 320));

    // Load font
    if (!font.loadFromFile("C:/Dev/2d_game_simple/src/data/fonts/Lato-Regular.ttf")) {
        std::cerr << "Error: Could not load font!" << std::endl;
    }

    // Set up title text
    setupText(titleText, "Terminal", {10, 10}, 30);

    std::ofstream file("C:/Dev/2d_game_simple/saved_logs/loss_data_" + run_id + ".csv", std::ios::trunc);
    file << "Step,Loss\n";  // Add header
    file.close();

    // Initialize sections
    initializeSections();
}


// Initialize terminal sections
void Terminal::initializeSections() {
    // Define all 19 sections with placeholder elements

    //Label - "Headless Mode"
    Section section_one;
    section_one.bounds = sf::RectangleShape(sf::Vector2f(200, 50));
    section_one.bounds.setPosition(0, 0);
    section_one.bounds.setFillColor(sf::Color(100, 100, 100));  // Gray
    setupText(section_one.label, "Headless Mode", {20, 10});
    sections.push_back(section_one);

    //ON button
    Section section_two;
    section_two.bounds = sf::RectangleShape(sf::Vector2f(100, 50));
    section_two.bounds.setPosition(0, 50);
    section_two.bounds.setFillColor(headlessMode ? sf::Color(0, 255, 0) : sf::Color(100, 100, 100));  // Green if active
    setupText(section_two.label, "ON", {25, 60});
    section_two.onClick = [&]() {
        headlessMode = true; //Set headless mode to true
        sections[1].bounds.setFillColor(sf::Color(0, 255, 0)); //Turn On to Green
        sections[2].bounds.setFillColor(sf::Color(100, 100, 100)); //Turn Off to Gray
        std::cout << "Headless mode turned ON. -From terminal.cpp\n";
    };
    sections.push_back(section_two);

    //OFF button
    Section section_three;
    section_three.bounds = sf::RectangleShape(sf::Vector2f(100, 50));
    section_three.bounds.setPosition(100, 50);
    section_three.bounds.setFillColor(!headlessMode ? sf::Color(255, 0, 0) : sf::Color(100, 100, 100));  // Red if active, Light gray if not
    setupText(section_three.label, "OFF", {125, 60});
    section_three.onClick = [&]() {
        headlessMode = false; //Set headless mode to false
        sections[1].bounds.setFillColor(sf::Color(100, 100, 100)); //Turn On to Green
        sections[2].bounds.setFillColor(sf::Color(255, 0, 0)); //Turn Off to Gray
        std::cout << "Headless mode turned OFF. -from  terminal.cpp section 3.\n";
    };
    sections.push_back(section_three);

    //Acceleration Factor Label
    Section section_four;
    section_four.bounds = sf::RectangleShape(sf::Vector2f(300, 50));
    section_four.bounds.setPosition(250, 0);
    section_four.bounds.setFillColor(sf::Color(100, 100, 100));
    setupText(section_four.label, "Acceleration Factor - Adjustable", {260, 10}, 20);
    sections.push_back(section_four);

    //AccelerationFactor Modifiable Field
    Section section_five;
    section_five.bounds = sf::RectangleShape(sf::Vector2f(300, 50));
    section_five.bounds.setPosition(250, 50);
    section_five.bounds.setFillColor(sf::Color(21, 71, 52));  // dark green
    setupText(section_five.label, std::to_string(accelerationFactor), {375, 60});  // Default value as text, text should change based on what the value is. accelerationFactor should be linked to the float value
    //Handle user input
    section_five.onClick = [&, index = sections.size()]() {
        isEditing = true; //Set editing flag
        editingSectionIndex = index; //Store the index of this section
        sections[editingSectionIndex].bounds.setFillColor(sf::Color(255, 0, 0));
        currentInput.clear(); //Reset input buffer when starting a new edit - currently no relation?
        std::cout << "Editing acceleration factor...\n";
    };
    sections.push_back(section_five);

    //System FPS Label
    Section section_six;
    section_six.bounds = sf::RectangleShape(sf::Vector2f(200, 50));
    section_six.bounds.setPosition(600, 0);
    section_six.bounds.setFillColor(sf::Color(100, 100, 100)); // same light gray 
    setupText(section_six.label, "System FPS", {640, 10});
    sections.push_back(section_six);

    //System FPS live updates
    Section section_seven;
    section_seven.bounds = sf::RectangleShape(sf::Vector2f(200, 50));
    section_seven.bounds.setPosition(600, 50);
    section_seven.bounds.setFillColor(sf::Color(71, 43, 18)); //brown
    //initial label text (e.g. "FPS: 31")
    setupText(section_seven.label, "0", {690, 60}); //come back to change "0" into the dynamic fps
    systemFPSIndex = sections.size(); //storing the index of this section
    sections.push_back(section_seven);

    //Effective FPS Label
    Section section_eight;
    section_eight.bounds = sf::RectangleShape(sf::Vector2f(200, 50));
    section_eight.bounds.setPosition(800, 0);
    section_eight.bounds.setFillColor(sf::Color(100, 100, 100));
    setupText(section_eight.label, "Effective FPS", {835, 10});
    sections.push_back(section_eight);

    //Effective FPS live updates
    Section section_nine;
    section_nine.bounds = sf::RectangleShape(sf::Vector2f(200, 50));
    section_nine.bounds.setPosition(800, 50);
    section_nine.bounds.setFillColor(sf::Color(71, 43, 18));
    //Initial label text (e.g. "31")
    setupText(section_nine.label, "0", {875, 60});
    effectiveFPSIndex = sections.size(); //storing the index of this section
    sections.push_back(section_nine);

    // Section 10: Replay Buffer Capacity Label
    Section section_ten;
    section_ten.bounds = sf::RectangleShape(sf::Vector2f(250, 75)); // (width=250, height=75) 
    section_ten.bounds.setPosition(0, 125);
    section_ten.bounds.setFillColor(sf::Color(100, 100, 100));  // Gray background
    setupText(section_ten.label, "Replay Buffer Capacity", {17, 145}, 22); 
    sections.push_back(section_ten);

    // Section 11: replay buffer x/100,000 -> link up exactly like the fps, in chatgpt
    Section section_eleven;
    section_eleven.bounds = sf::RectangleShape(sf::Vector2f(250, 75));
    section_eleven.bounds.setPosition(250, 125);
    section_eleven.bounds.setFillColor(sf::Color(71, 43, 18));
    //Initial label text
    setupText(section_eleven.label, "x/100,000", {295, 145}, 22);
    replayBufferIndex = sections.size(); //storing the index of this section
    sections.push_back(section_eleven);
    
    //session runtime label
    Section section_twelve;
    section_twelve.bounds = sf::RectangleShape(sf::Vector2f(250, 75));
    section_twelve.bounds.setPosition(500, 125);
    section_twelve.bounds.setFillColor(sf::Color(100, 100, 100));
    setupText(section_twelve.label, "Session Runtime", {520, 145});
    sections.push_back(section_twelve);

    //Section 13, clock in 00h 00m 00s format
    Section section_thirteen;
    section_thirteen.bounds = sf::RectangleShape(sf::Vector2f(250, 75));
    section_thirteen.bounds.setPosition(750, 125);
    section_thirteen.bounds.setFillColor(sf::Color(71, 43, 18)); //brown
    //Initial label text
    setupText(section_thirteen.label, "00h 00m 00s", {800, 145});
    sessionRuntimeIndex = sections.size(); //storing the index of this section
    sections.push_back(section_thirteen);

    //Section 14 - Cumulative Reward by Episode GRAPH
    Section section_fourteen;
    section_fourteen.bounds = sf::RectangleShape(sf::Vector2f(500, 500));
    section_fourteen.bounds.setPosition(0, 225);
    section_fourteen.bounds.setFillColor(sf::Color(50, 50, 50)); //shouldnt actually see this - image will overlay
    cumulativeRewardGraphIndex = sections.size();
    sections.push_back(section_fourteen);

    Section section_fifteen;
    section_fifteen.bounds = sf::RectangleShape(sf::Vector2f(500, 500));
    section_fifteen.bounds.setPosition(500, 225);
    section_fifteen.bounds.setFillColor(sf::Color(50, 50, 50));
    lossGraphIndex = sections.size();
    sections.push_back(section_fifteen);

    Section section_sixteen;
    section_sixteen.bounds = sf::RectangleShape(sf::Vector2f(250, 50));
    section_sixteen.bounds.setPosition(0, 750);
    section_sixteen.bounds.setFillColor(sf::Color(0, 0, 128)); //navy blue
    setupText(section_sixteen.label, "Save Policy Network", {15, 760});
    //Defining the onClick functionality
    section_sixteen.onClick = [&]() {
        std::string save_dir = "C:/Dev/2d_game_simple/saved_models";
        std::string model_name = "policy_network_" + run_id + ".pt";  // Generate unique name
        std::string full_path = save_dir + "/" + model_name;
        training_loop.save_model(policy_network, save_dir, run_id, global_frame_counter);
        //confirmation message
        std::cout << "Model saved successfully at: " << full_path << "\n";
        };
    sections.push_back(section_sixteen);

    // export graphs button
    Section section_seventeen;
    section_seventeen.bounds = sf::RectangleShape(sf::Vector2f(250, 50));
    section_seventeen.bounds.setPosition(300, 750);
    section_seventeen.bounds.setFillColor(sf::Color(0, 0, 128)); //navy blue
    setupText(section_seventeen.label, "Export Graphs", {355, 760});

    // Defining the onClick functionality
    section_seventeen.onClick = [&]() {
        // Unique file paths using the global frame counter
        std::string reward_graph_path = "C:/Dev/2d_game_simple/saved_logs/reward_graph_" 
            + std::to_string(global_frame_counter) + ".png";
        std::string loss_graph_path = "C:/Dev/2d_game_simple/saved_logs/loss_graph_" 
            + std::to_string(global_frame_counter) + ".png";

        // 1) Reward graph via logger
        logger.saveAndPlotRewards(run_id, reward_graph_path);
        // 2) Loss graph via training_loop
        std::string loss_data_path = "C:/Dev/2d_game_simple/saved_logs/loss_data_" + run_id + ".csv";
        training_loop.save_loss_to_csv(loss_data_path, loss_step_counter);
        std::string command = "\"C:/Users/hanse/AppData/Local/Programs/Python/Python312/python.exe\" "
                        "C:/Dev/2d_game_simple/python_logging/generate_plot.py loss " 
                        + loss_data_path + " " + loss_graph_path;
        system(command.c_str());

        loss_step_counter++;

        std::cout << "Graphs exported successfully to unique names!\n";
    };


    sections.push_back(section_seventeen);

    // placedholder button
    Section section_eighteen;
    section_eighteen.bounds = sf::RectangleShape(sf::Vector2f(100, 50));
    section_eighteen.bounds.setPosition(600, 750);
    section_eighteen.bounds.setFillColor(sf::Color(128, 128, 0));
    setupText(section_eighteen.label, "Placeholder", {620, 760}, 14);
    sections.push_back(section_eighteen);


    // Section 19: End Session Button
    Section section_nineteen;
    section_nineteen.bounds = sf::RectangleShape(sf::Vector2f(250, 50));
    section_nineteen.bounds.setPosition(750, 750);
    section_nineteen.bounds.setFillColor(sf::Color(255, 0, 0));  // Red
    setupText(section_nineteen.label, "End Session", {775, 760});

    section_nineteen.onClick = [&]() {
        std::cout << "Ending session...\n";


        window.close();  // Close the window (similar to ESC behavior)
    };
    sections.push_back(section_nineteen);

    // Define remaining sections similarly...
}





//
//

// Handle events
void Terminal::handleEvents() {
    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            window.close();
        }
        else if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
        //Handle mouse clicks
        sf::Vector2i mousePosition = sf::Mouse::getPosition(window);
        handleClick(mousePosition);
    }
        else if (isEditing && event.type == sf::Event::TextEntered) {
            if (editingSectionIndex < 0 || editingSectionIndex >= (int)sections.size()) {
                //Safety Check
                std::cerr << "Invalid editingSectionIndex!\n";
                isEditing = false;
                editingSectionIndex = -1;
                currentInput.clear();
                continue;
            }

            if (event.text.unicode == '\r') {
                //If ENTER key is hit
                try {
                    float new_acceleration = std::stof(currentInput); //come back when we update currentInput
                    if (new_acceleration > 0) {
                        accelerationFactor = new_acceleration;
                        sections[editingSectionIndex].label.setString(std::to_string(accelerationFactor));
                        std::cout << "New acceleration factor set to: " << accelerationFactor << "\n";
                    } else {
                        std::cerr << "Acceleration factor must be positive.\n";
                    }
                } catch (...) {
                    std::cerr << "Invalid input for unknown reason. Resetting.\n";
                }
                //Reset Editing state after Enter key is hit
                sections[editingSectionIndex].bounds.setFillColor(sf::Color(150, 150, 150));
                isEditing = false;
                editingSectionIndex = -1;
                currentInput.clear();
            }
            else if (event.text.unicode == '\b') {
                //If backspace is hit
                if (!currentInput.empty()) {
                    currentInput.pop_back();
                }
            }
            else if (event.text.unicode >= 32 && event.text.unicode <= 126) {
                //Valid ASCII, includes way more  than just digits. guess we could tighten this up but no need.
                currentInput += static_cast<char>(event.text.unicode);
            }
            //Update the label in real time
            if (editingSectionIndex >= 0 && editingSectionIndex < (int)sections.size()) {
                sections[editingSectionIndex].label.setString(currentInput.empty() ? " " : currentInput);
            }
        }
    }
}


// Update terminal (placeholder for now)
void Terminal::update() {
    static float timeAccumulator = 0.0f;
    static float graphTimeAccumulator = 0.0f;
    static sf::Clock clock;

    float elapsed = clock.restart().asSeconds();
    timeAccumulator += elapsed;
    graphTimeAccumulator += elapsed; //seperate timer for both graphs
    //static size_t step = 0; //increment steps for the loss graph


        //get replay buffer capacity
    size_t currentSize = replayBuffer.size();
    if (replayBufferIndex >= 0 && replayBufferIndex < (int)sections.size()) {
        std::string displayString = std::to_string(currentSize) + " / " + std::to_string(bufferCapacity);
        sections[replayBufferIndex].label.setString(displayString);
    }

    //Only update once per second
    if (timeAccumulator >= 1.0f) {
        timeAccumulator = 0.0f;
        //get system fps
        int fps = static_cast<int>(utility.get_system_FPS());
        //Update the label for section 7
        if (systemFPSIndex >=0 && systemFPSIndex < (int)sections.size()) {
            sections[systemFPSIndex].label.setString(std::to_string(fps));
        }

        //get effective fps
        int eff_fps = static_cast<int>(utility.get_effective_FPS());
        if (effectiveFPSIndex >= 0 && effectiveFPSIndex < (int)sections.size()) {
            sections[effectiveFPSIndex].label.setString(std::to_string(eff_fps));
        }



        //convert totalTimeInSeconds to hours:minutes:seconds
        int totalSec = static_cast<int>(totalTimeInSeconds);
        int hours = totalSec/3600;
        int minutes = (totalSec%3600) / 60;
        int seconds = (totalSec%3600) % 60;
        std::string runtimeStr = std::to_string(hours) + "h " + std::to_string(minutes) + "m " + std::to_string(seconds) + "s";
        //Update the label if index is valid
        if (sessionRuntimeIndex >= 0 && sessionRuntimeIndex < (int)sections.size()) {
            sections[sessionRuntimeIndex].label.setString(runtimeStr);
        }

//...update()
        //GRAPH UPDATES
        const float graphUpdateInterval = 60.0f;  // Update graph every x seconds seconds. 5 for debugging, 60+ for training.
        if (graphTimeAccumulator >= graphUpdateInterval) {
            graphTimeAccumulator = 0.0f;

            // Update cumulative reward graph
            updateCumulativeRewardGraph(logger, run_id, global_frame_counter);
            updateLossGraph(training_loop, run_id, loss_step_counter++);
        }
    }
}

void Terminal::updateCumulativeRewardGraph(Logger& logger, const std::string& run_id, size_t global_frame_counter) {
    // Step 1: Define a unique output path for the reward graph using the global frame counter
    std::string reward_graph_path = "C:/Dev/2d_game_simple/saved_logs/reward_plot_" + std::to_string(global_frame_counter) + ".png";

    // Step 2: Save rewards and generate the new graph using the updated logger function
    logger.saveAndPlotRewards(run_id, reward_graph_path);

    // Step 3: Load the updated graph image
    loadCumulativeRewardGraph(reward_graph_path);
}



void Terminal::loadCumulativeRewardGraph(const std::string& imagePath) {
    //Attempt to load the graph image
    if (!cumulativeRewardTexture.loadFromFile(imagePath)) {
        std::cerr << "Failed to load graph image from: " << imagePath << "check terminal.cpp \n";
        return;
    }
    cumulativeRewardSprite.setTexture(cumulativeRewardTexture);
   
    cumulativeRewardSprite.setPosition(0.f, 225.f);
}

void Terminal:: updateLossGraph(TrainingLoop& training_loop, const std::string& run_id, size_t step) {
    // Save the averaged loss for the current step
    std::string lossCSVPath = "C:/Dev/2d_game_simple/saved_logs/loss_data_" + run_id + ".csv";
    training_loop.save_loss_to_csv(lossCSVPath, step);

    // Call the Python script to generate the graph
    std::string lossImagePath = "C:/Dev/2d_game_simple/saved_logs/loss_plot_" + run_id + ".png";
    std::string command = "\"C:/Users/hanse/AppData/Local/Programs/Python/Python312/python.exe\" "
                          "C:/Dev/2d_game_simple/python_logging/generate_plot.py loss " +
                          lossCSVPath + " " + lossImagePath;
    system(command.c_str());
    // Load the updated graph
    loadLossGraph(lossImagePath);
}

void Terminal::loadLossGraph(const std::string& imagePath) {
    if (!lossGraphTexture.loadFromFile(imagePath)) {
        std::cerr << "Failed to load loss graph image from: " << imagePath << "check terminal.cpp \n";
        return;
    }
    lossGraphSprite.setTexture(lossGraphTexture);
    lossGraphSprite.setPosition(500.0f, 225.0f); //position for the loss graph on the terminal
}


//Handle Clicks
void Terminal::handleClick(sf::Vector2i mousePosition) {
    for (size_t i = 0; i < sections.size(); i++) {
        auto& section = sections[i];
        if (section.bounds.getGlobalBounds().contains(static_cast<sf::Vector2f>(mousePosition))) {
            if (section.onClick) {
                section.onClick();
            }
        }
    }
}

// Render terminal
void Terminal::render() {
    window.clear(sf::Color(50, 50, 50));  // Gray background
    drawGrid();  // Draw the faint grid

    // Draw sections
    for (auto& section : sections) {
        window.draw(section.bounds);
        window.draw(section.label);
    }

    window.draw(cumulativeRewardSprite);
    window.draw(lossGraphSprite);
    window.display();
}

// Draw faint grid
void Terminal::drawGrid() {
    sf::Color gridColor(200, 200, 200, 50);  // Very faint gray
    for (int x = 0; x <= 1000; x += 25) {
        sf::Vertex line[] = {
            sf::Vertex(sf::Vector2f(x, 0), gridColor),
            sf::Vertex(sf::Vector2f(x, 800), gridColor)
        };
        window.draw(line, 2, sf::Lines);
    }

    for (int y = 0; y <= 800; y += 25) {
        sf::Vertex line[] = {
            sf::Vertex(sf::Vector2f(0, y), gridColor),
            sf::Vertex(sf::Vector2f(1000, y), gridColor)
        };
        window.draw(line, 2, sf::Lines);
    }
}

// Utility to set up text properties
void Terminal::setupText(sf::Text& text, const std::string& str, sf::Vector2f position, unsigned int size) {
    text.setFont(font);
    text.setString(str);
    text.setCharacterSize(size);
    text.setFillColor(sf::Color::White);
    text.setPosition(position);
}
