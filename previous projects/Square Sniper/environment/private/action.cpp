#include "action.hpp"
#include <random>

// Constructor definition.
Action::Action(float dx, float dy, int s)
    : delta_x(dx), delta_y(dy), shoot(s)
{
}

// Returns a flat array representation of the action.
std::vector<float> Action::toFlatArray() const {
    return { delta_x, delta_y, static_cast<float>(shoot) };
}

Action generateRandomAction() {
    // Static random number generator for reproducibility across calls.
    static std::mt19937 rng{ std::random_device{}() };

    // Uniform distribution for continuous actions in the range [-1, 1].
    std::uniform_real_distribution<float> continuous_dist(-1.f, 1.f);
    // Uniform distribution for the discrete action (0 or 1).
    std::uniform_int_distribution<int> discrete_dist(0, 1);

    float dx = continuous_dist(rng);
    float dy = continuous_dist(rng);
    int s = discrete_dist(rng);

    return Action(dx, dy, s);
}
