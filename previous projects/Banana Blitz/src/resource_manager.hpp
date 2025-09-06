#ifndef RESOURCE_MANAGER_HPP
#define RESOURCE_MANAGER_HPP

#include <SFML/Graphics.hpp>
#include <iostream>

class ResourceManager
{
public:
    // Constructor
    ResourceManager();

    // Load all textures
    bool load_textures();

    // Get texture reference
    const sf::Texture &get_main_texture() const;

private:
    sf::Texture main_texture;
};

#endif // RESOURCE_MANAGER_HPP
