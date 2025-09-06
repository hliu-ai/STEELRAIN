#pragma once
#include <SFML/Graphics.hpp>

class Crosshair {
public:
    // Constructor taking a float, with a default value.
    Crosshair(float size = 10.f);

    void updatePosition(const sf::Vector2f& pos);
    void draw(sf::RenderWindow& window) const;

private:
    sf::VertexArray lines;
    float size;
};
