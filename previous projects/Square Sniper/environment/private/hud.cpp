#include "hud.hpp"
#include <iostream>
#include <cstdlib> // For exit()

HUD::HUD(int maxShots, float episodeDurationSeconds)
    : maxShots(maxShots), shotsRemainingCount(maxShots), episodeDuration(episodeDurationSeconds)
{
    if (!font.loadFromFile("C:\\Windows\\Fonts\\arial.ttf")) {
        std::cerr << "Error loading font!\n";
        exit(EXIT_FAILURE);
    }
    timerText.setFont(font);
    timerText.setCharacterSize(20);
    timerText.setFillColor(sf::Color::Yellow);
    timerText.setPosition(10.f, 10.f);

    shotsText.setFont(font);
    shotsText.setCharacterSize(20);
    shotsText.setFillColor(sf::Color::Yellow);
    shotsText.setPosition(350.f, 10.f);
}

void HUD::update(sf::RenderWindow& window) {
    float timeRemaining = episodeDuration - episodeClock.getElapsedTime().asSeconds();
    timerText.setString("Time: " + std::to_string(static_cast<int>(timeRemaining)));
    shotsText.setString("Shots: " + std::to_string(shotsRemainingCount) + "/" + std::to_string(maxShots));
}

void HUD::draw(sf::RenderWindow& window) {
    window.draw(timerText);
    window.draw(shotsText);
}

void HUD::decrementShots() {
    shotsRemainingCount--;
}

bool HUD::isEpisodeOver() const {
    return shotsRemainingCount == 0 || episodeClock.getElapsedTime().asSeconds() >= episodeDuration;
}

void HUD::reset() {
    shotsRemainingCount = maxShots;
    episodeClock.restart();
}

int HUD::shotsRemaining() const {
    return shotsRemainingCount;
}
