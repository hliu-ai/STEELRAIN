#include "raycasting.hpp"
#include <iostream>
#include <cmath>
#include <string>
#include <iomanip>

// Constructor for Raycasting class; initializes an instance of the Raycasting class.
Raycasting::Raycasting() {}

// Main function to perform raycasting.
void Raycasting::perform_raycasting(
    const sf::Vector2f &origin,                   // Starting point of the raycasting (the origin of rays).
    const sf::Vector2f &direction,                // Direction vector indicating where the rays are pointing.
    float vision_angle,                           // Total angle (in degrees) of the vision cone.
    int num_rays,                                 // Number of rays to cast within the vision cone.
    float max_ray_length,                         // Maximum length each ray can travel.
    const std::vector<environmental_object> &platforms, // List of platforms (potential obstacles).
    const std::vector<std::unique_ptr<collectable>> &bananas) // List of collectable objects (bananas).
{
    rays.clear(); // Clear any previously stored rays.
    ray_collisions.clear(); // Clear any previously recorded collision information.

    // Calculate the starting angle for the vision cone (half the vision angle to the left of the main direction).
    float start_angle = std::atan2(direction.y, direction.x) - (vision_angle / 2.0f) * (3.14159f / 180.0f);

    // Calculate the angle increment between each ray based on the vision angle and the number of rays.
    float angle_increment = vision_angle * (3.14159f / 180.0f) / num_rays;

    // Cast rays across the vision cone.
    for (int i = 0; i < num_rays; ++i)
    {
        // Calculate the current angle for this ray.
        float current_angle = start_angle + i * angle_increment;

        // Create a direction vector for the current ray using the angle.
        sf::Vector2f ray_direction(std::cos(current_angle), std::sin(current_angle));

        // Initialize the ray as a line (two points: start and end).
        sf::VertexArray ray(sf::Lines, 2);
        ray[0].position = origin; // Start point of the ray at the origin.
        ray[0].color = sf::Color::Red; // Set initial ray color to red.

        // Calculate the endpoint of the ray if it reaches its maximum length.
        sf::Vector2f end_point = origin + ray_direction * max_ray_length;

        ObjectType collision_type = ObjectType::None; // Default collision type is 'None' (no collision).
        float closest_distance = max_ray_length; // Set the initial closest distance to the max ray length.

        // Check for collisions with platforms and adjust ray if a collision occurs.
        for (const auto &platform : platforms)
        {
            // Get the bounding box of the platform.
            sf::FloatRect platform_bounds = platform.get_platform_bounds();
            sf::Vector2f intersection_point; // Point of intersection if a collision occurs.

            // Check if the ray intersects the platform's bounding box.
            if (rayIntersectsRectangle(origin, ray_direction, platform_bounds, intersection_point))
            {
                // Calculate the distance from the origin to the intersection point.
                float distance = std::hypot(intersection_point.x - origin.x, intersection_point.y - origin.y);

                // If this intersection is closer than any previous ones, update the ray's endpoint.
                if (distance < closest_distance)
                {
                    closest_distance = distance;
                    end_point = intersection_point; // Adjust the endpoint to the intersection point.
                    collision_type = ObjectType::Platform; // Set collision type to 'Platform'.
                }
            }
        }

        // Check for collisions with bananas.
        for (const auto &banana : bananas)
        {
            // Get the bounding box of the banana.
            sf::FloatRect banana_bounds = banana->get_collectable_sprite().getGlobalBounds();
            sf::Vector2f intersection_point; // Point of intersection if a collision occurs.

            // Check if the ray intersects the banana's bounding box.
            if (rayIntersectsRectangle(origin, ray_direction, banana_bounds, intersection_point))
            {
                // Calculate the distance from the origin to the intersection point.
                float distance = std::hypot(intersection_point.x - origin.x, intersection_point.y - origin.y);

                // If this intersection is closer than any previous ones, update the ray's endpoint.
                if (distance < closest_distance)
                {
                    closest_distance = distance;
                    end_point = intersection_point; // Adjust the endpoint to the intersection point.
                    collision_type = ObjectType::Banana; // Set collision type to 'Banana'.
                    ray[0].color = sf::Color::Green; // Change the ray color to green when colliding with a banana.
                }
            }
        }

        // Set the final position of the ray's endpoint.
        ray[1].position = end_point;
        ray[1].color = ray[0].color; // Ensure the ray's endpoint color matches the start point color.

        rays.push_back(ray); // Store the ray in a vector (important, used for rendering and input handling)
        ray_collisions.push_back(collision_type); //Stores the collision type in a vector, not encoded yet.

    /*
        // Debugging output: Print collision information to the console.
        std::string collision_object = (collision_type == ObjectType::Banana) ? "banana" : (collision_type == ObjectType::Platform) ? "platform" : "none";
        // Format output for better readability, adding leading zeros for single-digit ray numbers.
        std::string debug_i = std::to_string(i);
        if (i < 10) {
            debug_i = "0" + debug_i;
        }
        std::cout << "Ray number " << debug_i << " colliding with " << collision_object
                  << " at position (" << end_point.x << ", " << end_point.y << ") with distance " << closest_distance << std::endl; */
    }
    
}

// Function to check if a ray intersects a rectangle (e.g., platform or banana bounding box).
bool Raycasting::rayIntersectsRectangle(
    const sf::Vector2f &origin,                   // Starting point of the ray.
    const sf::Vector2f &direction,                // Direction vector of the ray.
    const sf::FloatRect &rect,                    // Bounding box of the object to check for intersection.
    sf::Vector2f &intersection)                   // Point of intersection (if any).
{
    // Calculate intersection with the rectangle's left and right edges.
    float tmin = (rect.left - origin.x) / direction.x;
    float tmax = ((rect.left + rect.width) - origin.x) / direction.x;

    // Ensure tmin is the smaller value.
    if (tmin > tmax)
        std::swap(tmin, tmax);

    // Calculate intersection with the rectangle's top and bottom edges.
    float tymin = (rect.top - origin.y) / direction.y;
    float tymax = ((rect.top + rect.height) - origin.y) / direction.y;

    // Ensure tymin is the smaller value.
    if (tymin > tymax)
        std::swap(tymin, tymax);

    // If there is no overlap between the x and y intersections, there is no intersection.
    if ((tmin > tymax) || (tymin > tmax))
        return false;

    // Update tmin to the closer of the y-intersections if applicable.
    if (tymin > tmin)
        tmin = tymin;

    // Update tmax to the closer of the y-intersections if applicable.
    if (tymax < tmax)
        tmax = tymax;

    // If tmin is negative, the intersection is behind the ray's origin, so ignore it.
    if (tmin < 0)
        return false;

    // Calculate the intersection point using the adjusted tmin.
    intersection = origin + direction * tmin;
    return true; // Intersection occurred.
}

// Draw all rays stored in the 'rays' vector to the given window.
void Raycasting::draw_rays(sf::RenderWindow &window) const
{
    for (const auto &ray : rays)
    {
        window.draw(ray); // Draw each ray (line segment) to the SFML window.
    }
}

// Return a reference to the map containing information about ray collisions.
const std::vector<ObjectType> &Raycasting::get_ray_collisions() const
{
    return ray_collisions; // VECTOR of ray index to collision type (e.g., 'Platform', 'Banana', 'None').
}


const std::vector<sf::VertexArray> &Raycasting::get_rays() const {
    return rays;  // Return the vector of rays.
}
