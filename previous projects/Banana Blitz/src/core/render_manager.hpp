// File: core/render_manager.hpp
#ifndef RENDER_MANAGER_HPP
#define RENDER_MANAGER_HPP

#include <SFML/Graphics.hpp>
#include "player.hpp"
#include "utility.hpp"
#include "episode_manager.hpp"

class RenderManager {
public:
    void draw_all(sf::RenderWindow &window, player_class &player, Utility &utility, EpisodeManager &episode_manager);
};

#endif // RENDER_MANAGER_HPP