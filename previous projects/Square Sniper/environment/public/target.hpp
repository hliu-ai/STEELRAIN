#pragma once
#include <SFML/Graphics.hpp>

class Target {
public:
    Target();
    void resetPosition();
    const sf::RectangleShape& getShape() const;
private:
    sf::RectangleShape shape;
};

