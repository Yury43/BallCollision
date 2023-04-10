#include <iostream>
#include <iomanip>
#include "engine.h"

#include <chrono>

#include "scenarios.h"
#include "disjoined_set_union.h"
#include "collision.h"
#include "collision_detector.h"
#include "profiler.h"


Engine::Engine()
{
    init_random(objects);
    // init_snooker(objects);
    
    //init_corner_bounce(objects);
    // init_chain(objects);
    // init_angled1(objects);
    // init_angled2(objects);
    // init_size(objects);
    
    for (const auto & item : objects)
    {
        objects_by_id[item->id] = item;
    }
}


std::vector<std::shared_ptr<Physical>> Engine::run_iteration(const float delta_time)
{
    PROFILE_NAMED("Engine::run_iteration");
    
    if (objects.empty())
    {
        return {};
    }
    
    if (MOVE_BEFORE_COLLISION)
    {
        move_objects(delta_time);
    }
    
    // Reduce time complexity by placing balls in a grid of bins, only balls in the same bin and its neighbors may interact

    {
        // QuadTree collision_detector(objects);
        LightQuadTree collision_detector(objects);
        
        std::vector<std::pair<uint32_t, uint32_t>> colliding_pairs;
        int collision_test_count = 0;
        
        {
            PROFILE_NAMED("Engine::run_iteration");
            for (const auto& item : objects)
            {
                // PROFILE_NAMED("Engine::run_iteration");
                // PROFILE();
                
                // Test and handle collision with walls 
                // Wall collisions have higher priority 
                if (!item->handle_wall_collision(0, 0, WINDOW_X, WINDOW_Y))
                {
                    // Test collision with other balls
                    collision_detector.detect_collisions(item.get(), colliding_pairs, &collision_test_count);
                }
            }
        }
        
        // Find collisions involving same balls 
        
        const int n_colliding_pairs = static_cast<int>(colliding_pairs.size());
        DisjoinedSetUnion colliding_sets(n_colliding_pairs);

        {
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
        }

        // If collision involves more than 2 balls, chose single pair randomly, leave others for later 

        std::vector<Collision> new_collisions;

        for (auto& [fst, snd] : colliding_sets.sets_by_parent())
        {
            int selected_pair_id = snd[rand_gen() % snd.size()];
        
            std::pair<int, int> selected_pair = colliding_pairs[selected_pair_id];

            new_collisions.push_back(Collision(
                objects_by_id[selected_pair.first],
                objects_by_id[selected_pair.second]
            ));
        }
        
        // Process new collisions and mark them accordingly 
        for (Collision& collision : new_collisions)
        {
            collision.handle();
        }

        // Apply new reactions
        for (auto& o : objects)
        {
            o->apply_reactions();
        }

        // if (false)
        {
            // Calculate total kinetic energy change to control accuracy of simulation 

            double total_energy = std::accumulate(objects.begin(), objects.end(), 0., [](double sum, const auto& item) {
                auto ball = dynamic_cast<Ball*>(item.get());
                return ball ? sum + ball->energy() : sum;
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
        move_objects(delta_time);
    }

    std::vector<std::shared_ptr<Physical>> ready_objects;
    ready_objects.reserve(objects.size());
    for (const auto & o : objects)
    {
        auto ball_ptr = dynamic_cast<Ball*>(o.get());
        if (ball_ptr)
            ready_objects.push_back(std::make_unique<Ball>(*ball_ptr));
    }
    
    return ready_objects;

}



void Engine::move_objects(const float deltaTime) const
{
    for (auto & item : objects)
    {
        item->update_position(deltaTime);
    }
}
