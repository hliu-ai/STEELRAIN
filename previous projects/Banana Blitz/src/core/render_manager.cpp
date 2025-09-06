#include "render_manager.hpp"

void RenderManager::draw_all(sf::RenderWindow &window, player_class &player, Utility &utility, EpisodeManager &episode_manager) {
    //Clear the window
    window.clear(sf::Color::White);
    // Draw platforms and bananas
    episode_manager.draw_all(window);

    // Draw player
    window.draw(player.get_player_sprite());

    // Draw the utility d-pad and timers and collision indicator
    utility.draw(window);

    // Draw rays for debugging
    player.get_raycasting().draw_rays(window);

    // Draw proximity radius for the player
    player.get_proximity_radius().draw(window);
}
