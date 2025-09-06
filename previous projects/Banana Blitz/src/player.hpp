#ifndef PLAYER_HPP
#define PLAYER_HPP

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include "resource_manager.hpp"
#include "raycasting.hpp"
#include "proximity_radius.hpp"
#include "object_type.hpp"
#include <vector>
#include <map>

using namespace std;
using namespace sf;

class environmental_object;
class collectable;
class ResourceManager;

class player_class
{
public:
    // Constructor
    player_class(ResourceManager &resource_manager);

    // Update and collision functions
    void update(bool move_up, bool move_down, bool move_left, bool move_right, float delta_time, const vector<environmental_object> &platforms, const vector<unique_ptr<collectable>> &bananas, float acceleration_factor);
    void handle_collision_platforms(const vector<environmental_object> &platforms);

    // Getter for player sprite
    const sf::Sprite &get_player_sprite() const;

    // Getter for y-position
    float get_y_position() const;

    // Getter for x-position
    float get_x_position() const;

    // Function to set player's position to spawn point
    void reset_position();

    // Getter functions for raycasting and proximity radius
    Raycasting& get_raycasting();
    const ProximityRadius& get_proximity_radius() const;

    // Draw functions
    void draw_proximity(sf::RenderWindow &window) const;

    bool get_is_colliding_with_platform() const;
    void set_is_colliding_with_platform(bool is_colliding);

private:
    sf::Sprite player_sprite;
    sf::Vector2f velocity;

    // Movement parameters
    const float acceleration;
    const float max_speed;
    const float friction;

    // Rays for visual debugging
    Raycasting raycasting;

    // Proximity radius for object detection
    ProximityRadius proximity_radius;  // Corrected: remove conflict with previous float definition

    bool is_colliding_with_platform;
};

#endif // PLAYER_HPP
