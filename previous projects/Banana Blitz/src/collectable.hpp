#ifndef COLLECTABLE_HPP
#define COLLECTABLE_HPP

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include "resource_manager.hpp"
#include <vector>
#include <memory>

using namespace std;
using namespace sf;

class collectable
{
public:
    collectable(ResourceManager &resource_manager);
    void set_position(float x, float y);
    void reset();
    bool is_collected(const Sprite &player_sprite);
    const sf::Sprite &get_collectable_sprite() const; // getter
    sf::SoundBuffer collect_sound_buffer;
    sf::Sound collect_sound;

    sf::Vector2f get_collectable_position() const;
    int get_bananas_collected() const;

    // Static functions for banana management
    static std::vector<sf::Vector2f> get_all_banana_positions();
    static std::vector<std::unique_ptr<collectable>> get_mega_level_bananas(ResourceManager &resource_manager);
    static std::vector<std::unique_ptr<collectable>> get_phase_one_bananas(ResourceManager &resource_manager);
    static std::vector<std::unique_ptr<collectable>> get_x_bananas(int x, ResourceManager &resource_manager);

private:
    sf::Sprite collectable_sprite;
    int bananas_collected;
};
#endif // COLLECTABLE_HPP
