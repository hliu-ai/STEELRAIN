#ifndef PROXIMITY_RADIUS_HPP
#define PROXIMITY_RADIUS_HPP

#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include "environmental_object.hpp"
#include "collectable.hpp"
#include "object_type.hpp" // Include ObjectType for consistency

// Struct to store detailed data about nearby objects
struct NearbyObjectData
{
    const sf::FloatRect* bounding_box; // Bounding box of the nearby object
    sf::Vector2f nearest_point;        // Nearest point on the object from the player
    sf::Vector2f direction_vector;     // Direction vector from player to nearest point
    float distance;                    // Distance to the nearest point
    ObjectType interaction_type;       // Type of interaction (None, Platform, Banana)
};

class ProximityRadius
{
public:
    ProximityRadius(float radius);

    // Update the radius position based on player's position and detect nearby objects
    void update(const sf::Vector2f &player_position, const std::vector<environmental_object> &platforms, const std::vector<std::unique_ptr<collectable>> &bananas);

    // Draw the radius circle and optionally highlight detected objects
    void draw(sf::RenderWindow &window) const;

    // Retrieve the list of nearby objects detected within the radius
    const std::vector<NearbyObjectData> &get_nearby_objects() const;

private:
    float radius;
    sf::CircleShape proximity_circle;
    std::vector<NearbyObjectData> nearby_objects;

    // Helper function to check distance between player and objects
    bool isWithinRadius(const sf::Vector2f &player_position, const sf::FloatRect &object_bounds) const;

    // Helper function to find the nearest point on the object's bounding box
    sf::Vector2f findNearestPoint(const sf::FloatRect &object_bounds, const sf::Vector2f &player_position) const;
};

#endif // PROXIMITY_RADIUS_HPP
