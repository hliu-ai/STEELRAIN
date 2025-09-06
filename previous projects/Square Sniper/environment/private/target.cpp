#include "target.hpp"
#include <random>

sf::Vector2f randomTargetPosition() {
    static std::mt19937 rng{ std::random_device{}() };
    std::uniform_int_distribution<int> dist(0, 450);
    return sf::Vector2f(static_cast<float>(dist(rng)), static_cast<float>(dist(rng)));
}

Target::Target() {
    shape.setSize(sf::Vector2f(50.f, 50.f));
    shape.setFillColor(sf::Color::White);
    resetPosition();
}

void Target::resetPosition() {
    shape.setPosition(randomTargetPosition());
}

const sf::RectangleShape& Target::getShape() const {
    return shape;
}
