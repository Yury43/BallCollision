#pragma once

#include <thread>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>
#include "engine.h"

class EngineWorker
{
public:

    EngineWorker(bool async_mode_);
    ~EngineWorker();
    void engine_loop();
    void run();
    std::vector<sf::CircleShape> get_update(float simDeltaTime);

private:

    const bool async_mode;
    Engine engine;

    std::thread engine_thread;
    std::atomic_bool run_flag = false;
    std::vector<sf::CircleShape> render_buffer;
    std::mutex render_buffer_mutex;

};

