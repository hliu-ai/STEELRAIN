#include "state_representation.hpp"
#include <cmath>
#include <iostream>

constexpr float PI = 3.14159265358979323846f;
static const float MAX_DISTANCE = std::sqrt(2.0f * 475.0f * 475.0f);

std::vector<float> generateStateArray(const sf::RectangleShape& target, const sf::Vector2f& crosshairPos) {
    std::vector<float> state;

    //Compute the target's center
    sf::FloatRect targetBounds = target.getGlobalBounds();
    sf::Vector2f targetCenter(targetBounds.left + targetBounds.width / 2, targetBounds.top + targetBounds.height / 2);
    
    // Compute the Euclidean distance from the crosshair to the target center.
    float dx = crosshairPos.x - targetCenter.x;
    float dy = crosshairPos.y - targetCenter.y;
    float distance_to_center = std::sqrt(dx * dx + dy * dy);

    // Compute the angle to the target center (in radians) using atan2
    float angle_to_target = std::atan2(dy, dx);

    // Compute the overlap indicator: 1 if the crosshair overlaps the target
    float overlap_indicator = targetBounds.contains(crosshairPos) ? 1.0f : 0.0f;

    

    //Math for 
    float norm_distance = 1.0f - (distance_to_center / MAX_DISTANCE);

    // Normalize the values (to be friendly to the network weights)
    float norm_crosshair_x = crosshairPos.x / 500.0f;
    float norm_crosshair_y = crosshairPos.y / 500.0f;
    norm_distance = std::clamp(norm_distance, 0.0f, 1.0f);
    float norm_angle = angle_to_target / static_cast<float>(PI);
    float norm_overlap = overlap_indicator;  // remains 0 or 1

    //Prepend the new state values.
    state.push_back(norm_crosshair_x);          // 1st value: Crosshair X position.
    state.push_back(norm_crosshair_y);          // 2nd value: Crosshair Y position.
    state.push_back(norm_distance);      // 3rd value: Euclidean distance.
    state.push_back(norm_angle);         // 4th value: Angle to target center.
    state.push_back(norm_overlap);       // 5th value: Overlap indicator.

    // Debug: print new state values. CONFIRMED FUNCTIONALITY - COMMENTED OUT.
    //std::cout << "Debug State: Crosshair (" << norm_crosshair_x << ", " << norm_crosshair_y << "), "
      //  << "Distance: " << norm_distance << ", Angle: " << norm_angle
        //<< ", Overlap: " << norm_overlap << std::endl;

    // Now, create the original 20x20 grid representation for the target.
    // (There are 400 grid points spaced every 25 pixels.)
    for (float y = 0.f; y < 500.f; y += 25.f) {
        for (float x = 0.f; x < 500.f; x += 25.f) {
            sf::Vector2f point(x, y);
            state.push_back(targetBounds.contains(point) ? 1.f : 0.f);
        }
    }

   

    return state;
}
