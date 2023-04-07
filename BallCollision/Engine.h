#pragma once

#include <vector>
#include <memory>
#include <random>
#include <unordered_map>
#include <SFML/Graphics/CircleShape.hpp>
#include "physicals.h"
#include "randgen.h"

// Time-obliviously manages handling of all the physics: state of balls, movement, grid, clipping testing and collision handling  

class Engine
{
public:
    Engine();

    std::vector<sf::CircleShape> run_iteration(float delta_time);

private:

    std::vector<std::shared_ptr<Ball>> balls;
    std::unordered_map<uint32_t, std::shared_ptr<Ball>> balls_by_id;
    
    float max_span = 0; // for grid separation 
    double total_energy_prev = -1; // kinetic energy for accuracy validation 
    double initial_energy = -1;
    
    std::mt19937 rand_gen = RandGen::get(); 

    void move_ball(Ball& ball, float deltaTime);
    void move_balls(float deltaTime); 
};
