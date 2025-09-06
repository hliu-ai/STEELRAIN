#include "player.hpp"
#include "environmental_object.hpp"
#include "collectable.hpp"
#include "proximity_radius.hpp" // Ensure the proximity radius is included
#include <SFML/Graphics.hpp>
#include <iostream>
#include <cmath>

player_class::player_class(ResourceManager &resource_manager) : acceleration(1200.0f), max_speed(400.0f), friction(1000.0f), proximity_radius(150.0f)
{
    // Use the texture from ResourceManager
    player_sprite.setTexture(resource_manager.get_main_texture());
    player_sprite.setTextureRect(sf::IntRect(0, 0, 50, 50)); // Example: set a 50x50 area for the player
    player_sprite.setPosition(500, 400); // Initial position
    is_colliding_with_platform = false; //for reward function

    velocity = sf::Vector2f(0.f, 0.f);
}

void player_class::update(bool move_up, bool move_down, bool move_left, bool move_right, float delta_time, const std::vector<environmental_object> &platforms, const std::vector<std::unique_ptr<collectable>> &bananas, float acceleration_factor)
{
    float adjusted_delta_time = delta_time * acceleration_factor;


    // Apply acceleration based on input
    if (move_up)
    {
        velocity.y -= acceleration * adjusted_delta_time;
        player_sprite.setTextureRect(sf::IntRect(50, 50, 50, 50)); // Update to "look up"
    }
    else if (move_down)
    {
        velocity.y += acceleration * adjusted_delta_time;
        player_sprite.setTextureRect(sf::IntRect(0, 100, 50, 50)); // Update to "look down"
    }
    if (move_left)
    {
        velocity.x -= acceleration * adjusted_delta_time;
        player_sprite.setTextureRect(sf::IntRect(0, 50, 50, 50)); // Update to "look left"
    }
    else if (move_right)
    {
        velocity.x += acceleration * adjusted_delta_time;
        player_sprite.setTextureRect(sf::IntRect(0, 0, 50, 50)); // Update to "look right"
    }

    // Clamp velocity to max speed
    if (std::hypot(velocity.x, velocity.y) > max_speed)
    {
        float scale = max_speed / std::hypot(velocity.x, velocity.y);
        velocity.x *= scale;
        velocity.y *= scale;
    }

    // Apply friction when no input is given
    if (!move_up && !move_down)
    {
        if (velocity.y > 0)
        {
            velocity.y -= friction * adjusted_delta_time;
            if (velocity.y < 0)
                velocity.y = 0;
        }
        else if (velocity.y < 0)
        {
            velocity.y += friction * adjusted_delta_time;
            if (velocity.y > 0)
                velocity.y = 0;
        }
    }

    if (!move_left && !move_right)
    {
        if (velocity.x > 0)
        {
            velocity.x -= friction * adjusted_delta_time;
            if (velocity.x < 0)
                velocity.x = 0;
        }
        else if (velocity.x < 0)
        {
            velocity.x += friction * adjusted_delta_time;
            if (velocity.x > 0)
                velocity.x = 0;
        }
    }

    // Update player position based on velocity
    player_sprite.move(velocity * adjusted_delta_time);

    // Handle collision with platforms
    handle_collision_platforms(platforms);

    sf::Vector2f origin;
    sf::Vector2f direction;

    sf::FloatRect player_bounds = player_sprite.getGlobalBounds();

    if (player_sprite.getTextureRect() == sf::IntRect(0, 0, 50, 50)) // Facing right
    {
        origin = sf::Vector2f(player_bounds.left + player_bounds.width, player_bounds.top + player_bounds.height / 2);
        direction = sf::Vector2f(1.f, 0.f);
    }
    else if (player_sprite.getTextureRect() == sf::IntRect(0, 50, 50, 50)) // Facing left
    {
        origin = sf::Vector2f(player_bounds.left, player_bounds.top + player_bounds.height / 2);
        direction = sf::Vector2f(-1.f, 0.f);
    }
    else if (player_sprite.getTextureRect() == sf::IntRect(50, 50, 50, 50)) // Facing up
    {
        origin = sf::Vector2f(player_bounds.left + player_bounds.width / 2, player_bounds.top);
        direction = sf::Vector2f(0.f, -1.f);
    }
    else if (player_sprite.getTextureRect() == sf::IntRect(0, 100, 50, 50)) // Facing down
    {
        origin = sf::Vector2f(player_bounds.left + player_bounds.width / 2, player_bounds.top + player_bounds.height);
        direction = sf::Vector2f(0.f, 1.f);
    }

    // Raycasting for vision
    raycasting.perform_raycasting(origin, direction, 120.0f, 20, 1500.0f, platforms, bananas);

    // Proximity detection update
    proximity_radius.update(player_sprite.getPosition(), platforms, bananas);
}

void player_class::reset_position()
{
    player_sprite.setPosition(500, 400); // Reset to initial position
    velocity = sf::Vector2f(0.f, 0.f); // Reset velocity
}

const sf::Sprite &player_class::get_player_sprite() const
{
    return player_sprite;
}

float player_class::get_y_position() const
{
    return player_sprite.getPosition().y;
}

float player_class::get_x_position() const{
    return player_sprite.getPosition().x;
}

void player_class::handle_collision_platforms(const std::vector<environmental_object> &platforms)
{
    set_is_colliding_with_platform(false); //for reward function
    //std::cout << "is_colliding? " << is_colliding_with_platform << endl;
    // In top-down view, collision with platforms will involve stopping movement in that direction
    for (const auto &platform : platforms)
    {
        if (player_sprite.getGlobalBounds().intersects(platform.get_platform_bounds()))
        {
            // Basic collision handling: Adjust position to prevent overlapping
            sf::FloatRect player_bounds = player_sprite.getGlobalBounds();
            sf::FloatRect platform_bounds = platform.get_platform_bounds();
            sf::Vector2f previous_position = player_sprite.getPosition();

            float player_bottom = player_bounds.top + player_bounds.height;
            float player_right = player_bounds.left + player_bounds.width;
            float platform_bottom = platform_bounds.top + platform_bounds.height;
            float platform_right = platform_bounds.left + platform_bounds.width;

            float overlap_left = player_right - platform_bounds.left;
            float overlap_right = platform_right - player_bounds.left;
            float overlap_top = player_bottom - platform_bounds.top;
            float overlap_bottom = platform_bottom - player_bounds.top;

            bool from_left = overlap_left < overlap_right && overlap_left < overlap_top && overlap_left < overlap_bottom;
            bool from_right = overlap_right < overlap_left && overlap_right < overlap_top && overlap_right < overlap_bottom;
            bool from_top = overlap_top < overlap_bottom && overlap_top < overlap_left && overlap_top < overlap_right;
            bool from_bottom = overlap_bottom < overlap_top && overlap_bottom < overlap_left && overlap_bottom < overlap_right;

            if (from_left)
            {
                set_is_colliding_with_platform(true); // for reward function
                //std::cout << "is_colliding? from player_class" << is_colliding_with_platform << endl;
                velocity.x = 0;
                player_sprite.setPosition(platform_bounds.left - player_bounds.width, previous_position.y);
            }
            else if (from_right)
            {
                set_is_colliding_with_platform(true); // for reward function
                //std::cout << "is_colliding? from player_class" << is_colliding_with_platform << endl;
                velocity.x = 0;
                player_sprite.setPosition(platform_bounds.left + platform_bounds.width, previous_position.y);
            }
            else if (from_top)
            {
                set_is_colliding_with_platform(true); // for reward function
                //std::cout << "is_colliding? from player_class" << is_colliding_with_platform << endl;
                velocity.y = 0;
                player_sprite.setPosition(previous_position.x, platform_bounds.top - player_bounds.height);
            }
            else if (from_bottom)
            {
                set_is_colliding_with_platform(true); // for reward function
                //std::cout << "is_colliding? from player_class" << is_colliding_with_platform << endl;
                velocity.y = 0;
                player_sprite.setPosition(previous_position.x, platform_bounds.top + platform_bounds.height);
            }
        }
    }
}

bool player_class::get_is_colliding_with_platform() const
{
    return is_colliding_with_platform;
}

void player_class::set_is_colliding_with_platform(bool is_colliding)
{
    is_colliding_with_platform = is_colliding;
}


Raycasting &player_class::get_raycasting()
{
    return raycasting;
}

void player_class::draw_proximity(sf::RenderWindow &window) const
{
    proximity_radius.draw(window);
}

const ProximityRadius &player_class::get_proximity_radius() const
{
    return proximity_radius;
}
