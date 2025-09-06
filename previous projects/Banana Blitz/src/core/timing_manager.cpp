#include "timing_manager.hpp"
#include "utility.hpp"

float TimingManager::update_timing_get_delta_time(Clock::time_point& last_frame_time, Clock::time_point start_time,
                                  float& total_time_in_seconds, Utility& utility, float acceleration_factor) {
    // Calculate current frame time
    auto current_frame_time = Clock::now();

    // Calculate delta time
    Duration elapsed_time = current_frame_time - last_frame_time;
    float delta_time = elapsed_time.count();

    // Calculate total runtime
    Duration total_runtime = current_frame_time - start_time;
    total_time_in_seconds = total_runtime.count();

    // Update timers in the Utility class
    utility.updateLifeClock(delta_time, acceleration_factor);
    utility.update_fps_counter(delta_time, acceleration_factor);

    // Update last frame time
    last_frame_time = current_frame_time;

    //Return delta time
    return delta_time;
}
