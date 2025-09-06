#ifndef TIMING_MANAGER_HPP
#define TIMING_MANAGER_HPP

#include <chrono>

class Utility; // Forward declaration of Utility class

class TimingManager {
public:
    using Clock = std::chrono::steady_clock;
    using Duration = std::chrono::duration<float>;

    // Function to handle timing updates
    float update_timing_get_delta_time(Clock::time_point& last_frame_time, Clock::time_point start_time,
                       float& total_time_in_seconds, Utility& utility, float acceleration_factor);
};

#endif // TIMING_MANAGER_HPP
