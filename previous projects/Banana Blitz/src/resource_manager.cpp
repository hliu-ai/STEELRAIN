#include "resource_manager.hpp"
#include <SFML/Graphics.hpp>
#include <iostream>

// Constructor
ResourceManager::ResourceManager() {}

// Load textures
bool ResourceManager::load_textures()
{
    if (!main_texture.loadFromFile("C:/Dev/2d_game_simple/src/data/images/spritesheet_200x200.png"))
    {
        std::cout << "Error loading main texture!" << std::endl;
        return false;
    }
    std::cout << "Main texture loaded successfully!" << std::endl;
    return true;
}

// Getter for the main texture
const sf::Texture &ResourceManager::get_main_texture() const
{
    return main_texture;
}
