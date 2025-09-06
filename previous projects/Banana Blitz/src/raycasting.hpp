#ifndef RAYCASTING_HPP
#define RAYCASTING_HPP

#include <SFML/Graphics.hpp>
#include <vector>
#include <map>
#include "environmental_object.hpp"
#include "collectable.hpp"
#include "object_type.hpp"

class Raycasting
{
public:
    Raycasting();
    void perform_raycasting(const sf::Vector2f &origin, const sf::Vector2f &direction, float vision_angle, int num_rays, float max_ray_length,
                            const std::vector<environmental_object> &platforms, const std::vector<std::unique_ptr<collectable>> &bananas);
    void draw_rays(sf::RenderWindow &window) const;
    const std::vector<ObjectType> &get_ray_collisions() const;

    const std::vector<sf::VertexArray> &get_rays() const;

private:
    bool rayIntersectsRectangle(const sf::Vector2f &origin, const sf::Vector2f &direction, const sf::FloatRect &rect, sf::Vector2f &intersection);

    std::vector<sf::VertexArray> rays; //Stores the rays cast (we have a getter for this)
    std::vector<ObjectType> ray_collisions; //maps each ray to its collision type
};

#endif // RAYCASTING_HPP
