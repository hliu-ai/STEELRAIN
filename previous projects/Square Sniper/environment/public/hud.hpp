#pragma once
#include <SFML/Graphics.hpp>

class HUD {
public:
    // Provide default parameters so that a default constructor is available.
    HUD(int maxShots = 30, float episodeDurationSeconds = 10.f);

    void update(sf::RenderWindow& window);
    void draw(sf::RenderWindow& window);
    void decrementShots();
    int shotsRemaining() const;
    void reset();
    bool isEpisodeOver() const;

private:
    sf::Font font;
    sf::Text timerText;
    sf::Text shotsText;
    sf::Clock episodeClock;

    int maxShots;
    int shotsRemainingCount;
    float episodeDuration;
};
