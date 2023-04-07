#include <iostream>
#include <iomanip>
#include <array>
#include <unordered_set>
#include "engine.h"

#include <cassert>

#include "scenarios.h"
#include "disjoined_set_union.h"
#include "collision.h"
#include "collision_detector.h"

sf::CircleShape ball_as_shape(const Ball& ball)
{
    sf::CircleShape shape;
    shape.setRadius(ball.R);
    shape.setPosition(ball.p.x - ball.R, ball.p.y - ball.R); // consider ball.p to be the center
    shape.setFillColor(ball.color);
    return shape;
}

Engine::Engine()
{
    init_random(balls);
    // init_snooker(balls);
    
    //init_corner_bounce(balls);
    // init_chain(balls);
    // init_angled1(balls);
    // init_angled2(balls);
    // init_size(balls);

    for (const auto& ball : balls)
    {
        balls_by_id[ball->id] = ball;
    }
}




std::vector<sf::CircleShape> Engine::run_iteration(const float delta_time)
{
    if (balls.empty())
    {
        return {};
    }
    
    if (MOVE_BEFORE_COLLISION)
    {
        move_balls(delta_time);
    }
    
    // Reduce time complexity by placing balls in a grid of bins, only balls in the same bin and its neighbors may interact

    {
        std::shared_ptr<CollisionDetector> collision_detector;
        
        // collision_detector = std::make_shared<OnlineQuadTree>();
        collision_detector = std::make_shared<OfflineQuadTree>(balls);
        // collision_detector = std::make_shared<BinGrid<Ball>>(balls);
        
        std::vector<std::pair<int, int>> colliding_pairs;

        int collision_test_count = 0;
        
        for (const auto& ball : balls)
        {
            // Test and handle collision with walls 
            // Wall collisions have higher priority 
            if (ball->test_and_handle_wall_collision(0, 0, WINDOW_X, WINDOW_Y))
            {
                continue;
            }

            // Test collision with other balls in bins 

            collision_detector->detect_collisions(ball, colliding_pairs, &collision_test_count);
        }
        
        // std::cout << "collision_test_count: " << collision_test_count << " / " << balls.size() * balls.size() << std::endl;
        
        // Find collisions involving same balls 

        const int n_colliding_pairs = static_cast<int>(colliding_pairs.size());
        DisjoinedSetUnion colliding_sets(n_colliding_pairs);

        for (int i = 0; i < n_colliding_pairs; ++i)
        {
            for (int j = i + 1; j < n_colliding_pairs; ++j)
            {
                if (colliding_pairs[i].first == colliding_pairs[j].first ||
                    colliding_pairs[i].first == colliding_pairs[j].second ||
                    colliding_pairs[i].second == colliding_pairs[j].first ||
                    colliding_pairs[i].second == colliding_pairs[j].second)
                {
                    colliding_sets.join(i, j);
                }
            }
        }

        // If collision involves more than 2 balls, chose single pair randomly, leave others for later 

        std::vector<Collision> new_collisions;
        

        // if (false)
        if (true) // schedule single random collision in a system
        {
            for (std::pair<const int, std::vector<int>>& collision_set : colliding_sets.sets_by_parent())
            {
                int selected_pair_id = collision_set.second[rand_gen() % collision_set.second.size()];
            
                std::pair<int, int> selected_pair = colliding_pairs[selected_pair_id];

                new_collisions.push_back(Collision(
                    balls_by_id[selected_pair.first],
                    balls_by_id[selected_pair.second]
                ));
            }
        }
        else // schedule all collisions
        {
            
            for (std::pair<const int, std::vector<int>>& collision_set : colliding_sets.sets_by_parent())
            {
                for (int selected_pair_id : collision_set.second)
                {
                    std::pair<int, int> selected_pair = colliding_pairs[selected_pair_id];
                    new_collisions.push_back(Collision(
                        balls_by_id[selected_pair.first],
                        balls_by_id[selected_pair.second]
                    ));
                }
            }
        }

        // Process new collisions and mark them accordingly 
        for (Collision& collision : new_collisions)
        {
            collision.handle();
            //collision.mark_started();
        }

        // Apply new reactions 
        for (auto& ball : balls)
        {
            ball->apply_reactions();
        }


        // if (false)
        {
            // Calculate total kinetic energy change to control accuracy of simulation 

            double total_energy = std::accumulate(balls.begin(), balls.end(), 0., [](float sum, const auto& ball) {
                return sum + ball->energy();
                //return sum + norm(ball->velocity()); 
                });

            if (initial_energy < 0)
            {
                initial_energy = total_energy;
            }

            if (total_energy_prev > 0)
            {
                double dE = total_energy - total_energy_prev;
                double dEp = dE / total_energy_prev * 100;
                if (std::abs(dEp) > 1e-1)
                {
                    //std::cout << std::showpos << std::fixed << std::setprecision(0) <<  << std::endl;
                    std::cout << std::fixed << std::setprecision(0)
                        << "dE " << std::setw(6) << std::showpos << dEp << " %"
                        << "\ttot E " << std::setw(6) << std::noshowpos << total_energy / initial_energy * 100 << " % "
                        << std::endl;
                }
            }
            total_energy_prev = total_energy;
        }
    }

    if (not MOVE_BEFORE_COLLISION)
    {
        move_balls(delta_time);
    }

    std::vector<sf::CircleShape> shapes;
    shapes.reserve(balls.size());
    for (const auto& ball : balls)
    {
        shapes.push_back(ball_as_shape(*ball));
    }

    return shapes;
}

void Engine::move_ball(Ball& ball, const float deltaTime)
{
    float dx = ball.dir.x * ball.speed * deltaTime;
    float dy = ball.dir.y * ball.speed * deltaTime;
    ball.p.x += dx;
    ball.p.y += dy;
}

void Engine::move_balls(const float deltaTime) 
{
    for (auto& ball : balls)
    {
        move_ball(*ball, deltaTime);
    }
}
