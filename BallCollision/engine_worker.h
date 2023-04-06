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

    explicit EngineWorker(bool async_mode_);
    
    EngineWorker(const EngineWorker &_) = delete;
    EngineWorker(EngineWorker &&_) = delete;
    EngineWorker operator =(const EngineWorker & _) = delete;
    EngineWorker operator =(const EngineWorker && _) = delete;
    
    ~EngineWorker();
    void engine_loop();
    void run();
    std::vector<sf::CircleShape> get_update(float sim_delta_time);

private:

    const bool async_mode;
    Engine engine;

    std::thread engine_thread;
    std::atomic_bool run_flag;
    std::vector<sf::CircleShape> render_buffer;
    std::mutex render_buffer_mutex;

};

