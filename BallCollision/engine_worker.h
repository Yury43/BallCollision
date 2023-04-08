#pragma once

#include <thread>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>
#include "engine.h"

// Manages the flow of simulation in synchronous or parallel mode  

class EngineWorker
{
public:

    explicit EngineWorker(bool async_mode_);
    
    EngineWorker(const EngineWorker &_) = delete;
    EngineWorker(EngineWorker &&_) = delete;
    EngineWorker operator =(const EngineWorker & _) = delete;
    EngineWorker operator =(const EngineWorker && _) = delete;
    
    ~EngineWorker();
    void run();
    void stop();
    bool is_running() const {return !async_mode || run_flag;}
    
    std::vector<std::shared_ptr<Physical>> get_update(float sim_delta_time);

private:

    const bool async_mode;
    Engine engine;

    std::thread engine_thread;
    std::atomic_bool run_flag;
    std::vector<std::shared_ptr<Physical>> render_buffer;
    std::mutex render_buffer_mutex;

    void engine_loop();
};

