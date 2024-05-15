#pragma once

#include <vector>
#include <memory>
#include <random>
#include <unordered_map>

#include "ball.h"
#include "physicals.h"
#include "randgen.h"

// Time-obliviously manages handling of all the physics: state of balls, movement, grid, clipping testing and collision handling  

class Engine
{
public:
    Engine();

    std::vector<std::pair<uint32_t, uint32_t>> brutforce();
    std::vector<std::shared_ptr<Physical>> run_iteration(float delta_time);

private:

    std::vector<std::shared_ptr<Ball>> agents;
    std::unordered_map<uint32_t, Ball*> objects_by_id;
    
    double total_energy_prev = -1; // kinetic energy for accuracy validation 
    double initial_energy = -1;
    
    std::mt19937 rand_gen = RandGen::get(); 
    
    void move_objects(float deltaTime); 
};
