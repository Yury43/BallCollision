#include <iostream>
#include "engine_worker.h"
#include "constants.h"

EngineWorker::EngineWorker(const bool async_mode_) : async_mode(async_mode_)
{

}

EngineWorker::~EngineWorker()
{
    run_flag = false;

    if (engine_thread.joinable())
    {
        engine_thread.join();
    }
}

void EngineWorker::engine_loop()
{
    sf::Clock clock;
    float last_time = clock.restart().asSeconds();
    float speed_up = 2;
    // calculate target iteration delay and next iteration time
    const auto iteration_delay = std::chrono::milliseconds(1000) / TARGET_FRAMERATE / speed_up;
    auto next_iteration_time = std::chrono::system_clock::now() + iteration_delay;
    std::cout << "physics_loop running" << std::endl;

    while (run_flag)
    {
        float current_time = clock.getElapsedTime().asSeconds();
        float deltaTime = (current_time - last_time) * speed_up;
        last_time = current_time;

        auto gballs = engine.run_iteration(deltaTime / speed_up);

        // Place shapes to draw in drawing buffer 
        {
            std::lock_guard<std::mutex> lock(render_buffer_mutex);
            render_buffer = std::move(gballs);
        }

        // Sleep the remaining time, reserved for iteration and calculate time of next iteration 
        std::this_thread::sleep_until(next_iteration_time);
        next_iteration_time = next_iteration_time + iteration_delay;

    }

    std::cout << "physics_loop done" << std::endl;
}

void EngineWorker::run()
{
    if (async_mode)
    {
        run_flag = true;
        engine_thread = std::thread(([this] {this->engine_loop(); }));
    }
}

std::vector<sf::CircleShape> EngineWorker::get_update(const float sim_delta_time)
{
    if (async_mode)
    {
        if (!run_flag)
        {
            throw std::logic_error("Can't get update, engine is not running!");
        }

        // get data to render from physics thread;
        {
            std::lock_guard<std::mutex> lock(render_buffer_mutex);
            return std::move(render_buffer);
        }
    }
    else
    {
        return engine.run_iteration(sim_delta_time);
    }
}
