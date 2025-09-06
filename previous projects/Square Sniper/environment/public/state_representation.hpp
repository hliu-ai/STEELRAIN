#pragma once
#include <vector>
#include <SFML/Graphics.hpp>

// Generates a 400-element state array (20x20 grid) for the current target state.
// Each element is 1 if the grid point is inside the target's bounds, and 0 otherwise.
// Updated to accept the crosshair position as a second argument.
std::vector<float> generateStateArray(const sf::RectangleShape& target, const sf::Vector2f& crosshairPos);
