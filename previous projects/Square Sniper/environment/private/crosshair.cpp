#include "crosshair.hpp"

Crosshair::Crosshair(float size)
    : lines(sf::Lines, 4), size(size) {
}

// Updates crosshair line positions based on provided coordinates
void Crosshair::updatePosition(const sf::Vector2f& pos) {
    lines[0].position = sf::Vector2f(pos.x - size, pos.y);
    lines[1].position = sf::Vector2f(pos.x + size, pos.y);
    lines[2].position = sf::Vector2f(pos.x, pos.y - size);
    lines[3].position = sf::Vector2f(pos.x, pos.y + size);

    for (int i = 0; i < 4; ++i)
        lines[i].color = sf::Color::Green;
}

// Draws crosshair lines to the window
void Crosshair::draw(sf::RenderWindow& window) const {
    window.draw(lines);
}
