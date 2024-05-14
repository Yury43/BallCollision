#include <iostream>
#include "engine_worker.h"
#include "constants.h"
#include "profiler.h"

EngineWorker::EngineWorker(const bool async_mode_) : async_mode(async_mode_)
{

}

EngineWorker::~EngineWorker()
{
    stop();
}

void EngineWorker::engine_loop()
{
    using namespace std::chrono_literals;
    sf::Clock clock;
    float last_time = clock.restart().asSeconds();
    
    // calculate target iteration delay and next iteration time
    const std::chrono::duration<double, std::micro> iteration_delay = 1000000us / TARGET_FRAMERATE / MAX_PHYS_SPEEDUP;
    
    auto next_iteration_time = std::chrono::steady_clock::now() + iteration_delay;
    std::cout << "physics_loop running" << std::endl;

    while (run_flag)
    {
        float current_time = clock.getElapsedTime().asSeconds();
        float deltaTime = (current_time - last_time) * MAX_PHYS_SPEEDUP;
        last_time = current_time;

        auto gballs = engine.run_iteration(deltaTime / MAX_PHYS_SPEEDUP);

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

void EngineWorker::stop()
{
    run_flag = false;

    if (engine_thread.joinable())
    {
        engine_thread.join();
    }
}

std::vector<std::shared_ptr<Collidable>> EngineWorker::get_update(const float sim_delta_time)
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
