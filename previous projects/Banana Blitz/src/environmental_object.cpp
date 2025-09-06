#include "environmental_object.hpp"
#include <vector>
#include <algorithm>
#include <random>

using namespace std;
using namespace sf;

environmental_object::environmental_object(float x, float y, float width, float height, sf::Color color):
    env_x_position(x), env_y_position(y) {
    shape.setSize(sf::Vector2f(width, height));
    shape.setFillColor(color);
    shape.setPosition(env_x_position, env_y_position);
}

void environmental_object::draw(sf::RenderWindow& window) const {
        window.draw(shape);
}

const sf::RectangleShape& environmental_object::get_shape() const {
    return shape;
}

//New function to get platform bounds (position and size)
sf::FloatRect environmental_object::get_platform_bounds() const {
    return shape.getGlobalBounds();
}

// Static function to get all possible platform positions
std::vector<std::tuple<float, float, float, float, sf::Color>> environmental_object::get_all_platform_positions() {
    return {
        {-20, -20, 1000, 20, sf::Color::Green},              // Top border
        {0, 800, 1000, 20, sf::Color::Green}, // Bottom border
        {-20, 0, 20, 800, sf::Color::Green},              // Left border
        {1000, 0, 20, 800, sf::Color::Green}, // Right border

        {50, 50, 50, 150, sf::Color::Blue},   // platform 1
        {100, 50, 50, 50, sf::Color::Blue},    //  platform 2
        {400, 0, 50, 200, sf::Color::Blue}, // platform 3
        {450, 150, 100, 50, sf::Color::Blue}, // platform 4
        {500, 200, 50, 100, sf::Color::Blue}, // platform 5
        {550, 250, 50, 50, sf::Color::Blue}, // platform 6
        {600, 0, 50, 200, sf::Color::Blue}, // platform 7
        {700, 250, 50, 50, sf::Color::Blue}, // platform 8
        {800, 150, 50, 50, sf::Color::Blue}, // platform 9
        {900, 0, 100, 100, sf::Color::Blue}, // platform 10
        {50, 300, 50, 150, sf::Color::Blue}, // platform 11
        {100, 350, 50, 50, sf::Color::Blue}, // platform 12
        {150, 300, 50, 150, sf::Color::Blue}, // platform 13
        {250, 300, 50, 150, sf::Color::Blue}, // platform 14
        {300, 400, 50, 50, sf::Color::Blue}, // platform 15
        {100, 550, 100, 100, sf::Color::Blue}, // platform 16
        {150, 650, 100, 100, sf::Color::Blue}, // platform 17
        {450, 600, 100, 50, sf::Color::Blue}, // platform 18
        {550, 550, 100, 50, sf::Color::Blue}, // platform 19
        {550, 650, 100, 50, sf::Color::Blue}, // platform 20
        {700, 500, 150, 50, sf::Color::Blue}, // platform 21
        {700, 700, 150, 50, sf::Color::Blue}  // platform 22
    };
}

// Static function to get phase one platform positions
std::vector<std::tuple<float, float, float, float, sf::Color>> environmental_object::get_phase_one_platform_positions() {
    return {
        {-20, -20, 1000, 20, sf::Color::Green},              // Top border
        {0, 800, 1000, 20, sf::Color::Green}, // Bottom border
        {-20, 0, 20, 800, sf::Color::Green},              // Left border
        {1000, 0, 20, 800, sf::Color::Green}, // Right border

        {500, 200, 50, 100, sf::Color::Blue}, // platform 5
        {550, 250, 50, 50, sf::Color::Blue}, // platform 6
        {700, 250, 50, 50, sf::Color::Blue}, // platform 8
        {700, 500, 150, 50, sf::Color::Blue}, // platform 21
        {550, 550, 100, 50, sf::Color::Blue}, // platform 19
        {250, 300, 50, 150, sf::Color::Blue}  // platform 14
    };
}

// Static function to get mega level platform positions
std::vector<std::tuple<float, float, float, float, sf::Color>> environmental_object::get_mega_level_platform_positions() {
    return get_all_platform_positions();
}

// Static function to get X random platforms, always including border platforms
std::vector<std::tuple<float, float, float, float, sf::Color>> environmental_object::get_x_platform_positions(int x) {
    // Get all platform positions and separate border platforms
    auto all_positions = get_all_platform_positions();
    std::vector<std::tuple<float, float, float, float, sf::Color>> border_platforms = {
        {-20, -20, 1000, 20, sf::Color::Green},              // Top border
        {0, 800, 1000, 20, sf::Color::Green}, // Bottom border
        {-20, 0, 20, 800, sf::Color::Green},              // Left border
        {1000, 0, 20, 800, sf::Color::Green}  // Right border
    };

    // Filter out border platforms from the rest
    std::vector<std::tuple<float, float, float, float, sf::Color>> non_border_platforms;
    std::copy_if(all_positions.begin(), all_positions.end(), std::back_inserter(non_border_platforms),
                 [&](const auto &platform) {
                     return std::find(border_platforms.begin(), border_platforms.end(), platform) == border_platforms.end();
                 });

    // Shuffle non-border platforms and select x of them
    std::shuffle(non_border_platforms.begin(), non_border_platforms.end(), std::default_random_engine(std::random_device{}()));
    if (x > non_border_platforms.size()) {
        x = non_border_platforms.size();
    }
    std::vector<std::tuple<float, float, float, float, sf::Color>> selected_platforms(non_border_platforms.begin(), non_border_platforms.begin() + x);

    // Combine border platforms with the selected platforms
    selected_platforms.insert(selected_platforms.end(), border_platforms.begin(), border_platforms.end());

    return selected_platforms;
}
