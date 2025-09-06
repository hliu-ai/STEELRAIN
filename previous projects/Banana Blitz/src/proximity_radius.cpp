#include "proximity_radius.hpp"
#include "player.hpp"
#include "environmental_object.hpp"
#include "collectable.hpp"
#include "object_type.hpp" // Include ObjectType for consistent encoding
#include <SFML/Graphics.hpp>
#include <cmath>
#include <iostream>

namespace {
    // Utility function to calculate the nearest point on the bounding box to the player
    sf::Vector2f findNearestPoint(const sf::FloatRect &object_bounds, const sf::Vector2f &player_position)
    {
        float nearest_x = std::max(object_bounds.left, std::min(player_position.x, object_bounds.left + object_bounds.width));
        float nearest_y = std::max(object_bounds.top, std::min(player_position.y, object_bounds.top + object_bounds.height));
        return sf::Vector2f(nearest_x, nearest_y);
    }

    // Utility function to check if an object is within a given radius
    bool isWithinRadius(const sf::Vector2f &player_position, const sf::FloatRect &object_bounds, float radius)
    {
        sf::Vector2f nearest_point = findNearestPoint(object_bounds, player_position);
        float distance = std::hypot(nearest_point.x - player_position.x, nearest_point.y - player_position.y);
        return distance <= radius;
    }

    // Utility function to process nearby objects
    void processNearbyObject(const sf::Vector2f &centered_position, const sf::FloatRect &object_bounds, ObjectType interaction_type, float radius, std::vector<NearbyObjectData> &nearby_objects)
    {
        if (isWithinRadius(centered_position, object_bounds, radius))
        {
            NearbyObjectData data;
            data.bounding_box = &object_bounds;
            data.nearest_point = findNearestPoint(object_bounds, centered_position);
            data.direction_vector = data.nearest_point - centered_position;
            data.distance = std::hypot(data.direction_vector.x, data.direction_vector.y);
            data.interaction_type = interaction_type;

            nearby_objects.push_back(data);
        }
    }
}

ProximityRadius::ProximityRadius(float radius) : radius(radius)
{
    proximity_circle.setRadius(radius);
    proximity_circle.setFillColor(sf::Color(255, 0, 0, 50)); // More transparent red
    proximity_circle.setOrigin(radius, radius); // Center the circle to player's position
}

void ProximityRadius::update(const sf::Vector2f &player_position, const std::vector<environmental_object> &platforms, const std::vector<std::unique_ptr<collectable>> &bananas)
{
    // Update the circle's position to align with the center of the player sprite
    sf::Vector2f centered_position = sf::Vector2f(player_position.x + 25, player_position.y + 25); // Assuming player sprite is 50x50, adjust accordingly
    proximity_circle.setPosition(centered_position);

    // Clear the list of nearby objects
    nearby_objects.clear();

    // Process nearby platforms
    for (const auto &platform : platforms)
    {
        processNearbyObject(centered_position, platform.get_platform_bounds(), ObjectType::Platform, radius, nearby_objects);
    }

    // Process nearby bananas
    for (const auto &banana : bananas)
    {
        processNearbyObject(centered_position, banana->get_collectable_sprite().getGlobalBounds(), ObjectType::Banana, radius, nearby_objects);
    }

    /*    // Debugging output
    for (const auto &object : nearby_objects)
    {
        std::string interaction_type_str = (object.interaction_type == ObjectType::Banana) ? "banana" : (object.interaction_type == ObjectType::Platform) ? "platform" : "none";
        std::cout << interaction_type_str << " detected inside radius at position (" << object.nearest_point.x << ", " << object.nearest_point.y << ") with distance " << object.distance << std::endl;
    }
    */
}

const std::vector<NearbyObjectData> &ProximityRadius::get_nearby_objects() const
{
    return nearby_objects;
}

void ProximityRadius::draw(sf::RenderWindow &window) const
{
    // Draw the proximity circle
    window.draw(proximity_circle);

    // Optionally, draw highlights on detected nearby objects
    for (const auto &object : nearby_objects)
    {
        sf::RectangleShape highlight(sf::Vector2f(object.bounding_box->width, object.bounding_box->height));
        highlight.setPosition(object.bounding_box->left, object.bounding_box->top);
        highlight.setFillColor(sf::Color(0, 255, 0, 100)); // Semi-transparent green for highlighting
        window.draw(highlight);

        // Draw a ray from the player to the nearest point on each detected object
        sf::RectangleShape ray_line(sf::Vector2f(object.distance, 3)); // Thickness of 3 pixels
        ray_line.setPosition(proximity_circle.getPosition());
        ray_line.setRotation(atan2(object.nearest_point.y - proximity_circle.getPosition().y, object.nearest_point.x - proximity_circle.getPosition().x) * 180 / 3.14159);
        if (object.interaction_type == ObjectType::Banana) {
            ray_line.setFillColor(sf::Color(0, 255, 0, 150)); // Green ray for bananas
        } else {
            ray_line.setFillColor(sf::Color(0, 0, 255, 150)); // Blue ray for platforms
        }
        window.draw(ray_line);
    }
}
