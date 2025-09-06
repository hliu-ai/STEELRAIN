#include "collectable.hpp"
#include <iostream>
#include <SFML/Graphics.hpp>
#include "resource_manager.hpp"
#include <vector>
#include <memory>
#include <random>

using namespace std;
using namespace sf;

collectable::collectable(ResourceManager &resource_manager) : bananas_collected(0)
{
    // Use the texture from ResourceManager
    collectable_sprite.setTexture(resource_manager.get_main_texture());
    collectable_sprite.setTextureRect(sf::IntRect(50, 0, 50, 50)); // Banana's portion of the sheet

    if (!collect_sound_buffer.loadFromFile("C:/Dev/2d_game_simple/src/data/sounds/collectable_sound.ogg"))
    {
        cout << "Failed to load collect sound. Please check collectable.cpp" << endl;
    }

    collect_sound.setBuffer(collect_sound_buffer);
}

// Set position manually instead of spawning randomly
void collectable::set_position(float x, float y)
{
    collectable_sprite.setPosition(x, y);
}

void collectable::reset()
{
    bananas_collected = 0;
}

bool collectable::is_collected(const Sprite &player_sprite)
{
    if (collectable_sprite.getGlobalBounds().intersects(player_sprite.getGlobalBounds()))
    {
        bananas_collected++;
        //collect_sound.play(); //uncomment for coin collection sounds
        return true;
    }
    return false;
}

const sf::Sprite &collectable::get_collectable_sprite() const
{ // getter for sprite
    return collectable_sprite;
}

sf::Vector2f collectable::get_collectable_position() const
{
    return collectable_sprite.getPosition();
}

int collectable::get_bananas_collected() const
{
    return bananas_collected;
}

// Static function to get all possible banana positions
std::vector<sf::Vector2f> collectable::get_all_banana_positions() {
    return {
        {0, 0}, // Banana 1
        {500, 0}, // Banana 2
        {450, 50}, // Banana 3
        {550, 50}, // Banana 4
        {700, 50}, // Banana 5
        {500, 100}, // Banana 6
        {100, 150}, // Banana 7
        {250, 150}, // Banana 8
        {900, 250}, // Banana 9
        {0, 350}, // Banana 10
        {200, 350}, // Banana 11
        {400, 350}, // Banana 12
        {800, 350}, // Banana 13
        {650, 400}, // Banana 14
        {450, 500}, // Banana 15
        {300, 600}, // Banana 16
        {550, 600}, // Banana 17
        {750, 600}, // Banana 18
        {50, 700}, // Banana 19
        {900, 700}  // Banana 20
    };
}

// Static function to create bananas for the mega level
std::vector<std::unique_ptr<collectable>> collectable::get_mega_level_bananas(ResourceManager &resource_manager) {
    std::vector<std::unique_ptr<collectable>> bananas;
    for (const auto &position : get_all_banana_positions()) {
        auto banana = std::make_unique<collectable>(resource_manager);
        banana->set_position(position.x, position.y);
        bananas.push_back(std::move(banana));
    }
    return bananas;
}

// Static function to create bananas for phase one training
std::vector<std::unique_ptr<collectable>> collectable::get_phase_one_bananas(ResourceManager &resource_manager) {
    std::vector<sf::Vector2f> phase_one_positions = {
        {700, 50}, // Banana 5
        {200, 350}, // Banana 11
        {400, 350}, // Banana 12
        {800, 350}, // Banana 13
        {650, 400}, // Banana 14
        {450, 500}, // Banana 15
        {50, 700},  // Banana 19
        {900, 700}  // Banana 20
    };

    std::vector<std::unique_ptr<collectable>> bananas;
    for (const auto &position : phase_one_positions) {
        auto banana = std::make_unique<collectable>(resource_manager);
        banana->set_position(position.x, position.y);
        bananas.push_back(std::move(banana));
    }
    return bananas;
}

// Static function to maintain x bananas on the screen
std::vector<std::unique_ptr<collectable>> collectable::get_x_bananas(int x, ResourceManager &resource_manager) {
    std::vector<std::unique_ptr<collectable>> bananas;
    std::vector<sf::Vector2f> all_positions = get_all_banana_positions();

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, all_positions.size() - 1);

    std::vector<bool> used_positions(all_positions.size(), false);

    for (int i = 0; i < x; ++i) {
        int index;
        do {
            index = dis(gen);
        } while (used_positions[index]);

        used_positions[index] = true;
        auto banana = std::make_unique<collectable>(resource_manager);
        banana->set_position(all_positions[index].x, all_positions[index].y);
        bananas.push_back(std::move(banana));
    }

    return bananas;
}