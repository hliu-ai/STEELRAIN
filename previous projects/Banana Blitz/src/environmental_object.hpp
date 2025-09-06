#ifndef ENVIRONMENTAL_OBJECT_HPP
#define ENVIRONMENTAL_OBJECT_HPP

#include <SFML/Graphics.hpp>
#include <vector>
#include <tuple>

class environmental_object
{
public:
    environmental_object(float x, float y, float width, float height, sf::Color color);
    void draw(sf::RenderWindow &window) const;

    // Add a getter for shape and bounds
    const sf::RectangleShape &get_shape() const;
    sf::FloatRect get_platform_bounds() const;

    // Static functions to get platform positions
    static std::vector<std::tuple<float, float, float, float, sf::Color>> get_all_platform_positions();
    static std::vector<std::tuple<float, float, float, float, sf::Color>> get_phase_one_platform_positions();
    static std::vector<std::tuple<float, float, float, float, sf::Color>> get_mega_level_platform_positions();
    static std::vector<std::tuple<float, float, float, float, sf::Color>> get_x_platform_positions(int x);

private:
    float env_x_position;
    float env_y_position;
    sf::RectangleShape shape;
};

#endif // ENVIRONMENTAL_OBJECT_HPP
