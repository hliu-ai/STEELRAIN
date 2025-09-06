#pragma once
#include <vector>

/// Represents an action for our environment, matching the Gym tuple:
/// Tuple( (Box(low=-1, high=1, shape=(2,)), Discrete(2) ) )
struct Action {
    float delta_x;  // Continuous horizontal adjustment (range: [-1, 1])
    float delta_y;  // Continuous vertical adjustment (range: [-1, 1])
    int shoot;      // Discrete shooting command: 0 (no shoot) or 1 (shoot)
    // ASK CHATGPT: should we be using int for the discrete action or some sort of bool for the gymnasium environment?

    // Constructor with default values.
    Action(float dx = 0.f, float dy = 0.f, int s = 0);

    // Converts the action into a flat array (vector of floats).
    // The array will have three elements: [delta_x, delta_y, shoot]
    std::vector<float> toFlatArray() const;
};

// Generates a random Action.
// Continuous values are drawn uniformly from [-1, 1].
// The shoot value is randomly chosen to be 0 or 1.
Action generateRandomAction();
