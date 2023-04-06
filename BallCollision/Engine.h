#pragma once

#include <vector>
#include <memory>
#include <unordered_map>
#include <SFML/Graphics/CircleShape.hpp>
#include "physicals.h"

class Engine
{
public:
    Engine();

    std::vector<sf::CircleShape> run_iteration(float deltaTime);

private:

    std::vector<std::shared_ptr<Ball>> balls;
    std::unordered_map<int, std::shared_ptr<Ball>> balls_by_id;
    float max_r = 0;
    double total_energy_prev = -1;
    double initial_enery = -1;

    void move_ball(Ball& ball, float deltaTime);
    void move_balls(float deltaTime);
};