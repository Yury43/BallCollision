#include <iostream>
#include <iomanip>
#include "engine.h"

#include <cassert>
#include <chrono>
#include <memory_resource>
#include <set>


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

template<typename T>
struct PairComp{
    inline bool operator()(const std::pair<uint32_t, uint32_t>& lhs, const std::pair<uint32_t, uint32_t>& rhs) const {
        if (lhs.first != rhs.first)
            return lhs.first < rhs.first;
        return lhs.second < rhs.second;
    }
};

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
        // LightQuadTree collision_detector;
        // QuadTreePvigier collision_detector(objects);
        QuadTree collision_detector;
        
        std::vector<std::pair<uint32_t, uint32_t>> colliding_pairs; 
        std::vector<std::pair<uint32_t, uint32_t>> colliding_pairs2;
        int collision_test_count = 0;
        
        {
            PROFILE_NAMED("Engine::run_iteration");
            for (int i = 0; i < objects.size(); ++i)
            {
                const auto& item = objects[i];
                // PROFILE_NAMED("Engine::run_iteration");
                // PROFILE();
                
                // Test and handle collision with walls 
                // Wall collisions have higher priority 
                if (!item->handle_wall_collision(0, 0, WINDOW_X, WINDOW_Y))
                {
                    // Test collision with other balls
                }
                collision_detector.detect_collisions(item.get(), i, colliding_pairs, &collision_test_count);

            }
        }

        if (false)
        {
            for (int i = 0; i < objects.size(); ++i)
            {
                for (int j = i + 1; j < objects.size(); ++j)
                {
                    if (j != i)
                    {
                        if (objects[i]->is_touching(objects[j].get()))
                        {
                            colliding_pairs2.push_back({
                                std::min(objects[i]->id, objects[j]->id),
                                std::max(objects[i]->id, objects[j]->id)
                            });
                        }
                    }
                }
            }

            if (colliding_pairs.size() != colliding_pairs2.size())
            {
                std::set a(colliding_pairs.begin(), colliding_pairs.end(), PairComp<uint32_t>());
                std::set b(colliding_pairs2.begin(), colliding_pairs2.end(), PairComp<uint32_t>());
                std::vector<std::pair<uint32_t, uint32_t>> diff_ab;
                std::vector<std::pair<uint32_t, uint32_t>> diff_ba;
                
                std::set_difference(a.begin(), a.end(), b.begin(), b.end(), std::back_inserter(diff_ab));
                std::set_difference(b.begin(), b.end(), a.begin(), a.end(), std::back_inserter(diff_ba));

                std::cout << "extra: " << std::endl;
                for (const auto & p : diff_ab)
                {
                    const Ball* b1 = dynamic_cast<const Ball*>(objects_by_id[p.first].get());
                    const Ball* b2 = dynamic_cast<const Ball*>(objects_by_id[p.second].get());
                    std::cout << b1->id << "x" << b2->id << "\t";
                    std::cout << "C1: " << b1->p << "\tR1: " << b1->R << "\t" ;
                    std::cout << "C2: " << b2->p << "\tR2: " << b2->R << "\t" ;
                    std::cout << "R12: " << (b1->R + b2->R) << "\t";
                    std::cout << "dist: " << dist(b1->p, b2->p) << " ";
                    std::cout << std::endl;
                }

                std::cout << "missing: " << std::endl;
                for (const auto & p : diff_ba)
                {
                    const Ball* b1 = dynamic_cast<const Ball*>(objects_by_id[p.first].get());
                    const Ball* b2 = dynamic_cast<const Ball*>(objects_by_id[p.second].get());
                    std::cout << b1->id << "x" << b2->id << "\t";
                    std::cout << "C1: " << b1->p << "\tR1: " << b1->R << "\t" ;
                    std::cout << "C2: " << b2->p << "\tR2: " << b2->R << "\t" ;
                    std::cout << "R12: " << (b1->R + b2->R) << "\t";
                    std::cout << "dist: " << dist(b1->p, b2->p) << " ";
                    std::cout << std::endl;
                }

                assert(colliding_pairs.size() == colliding_pairs2.size());
            }
            
            assert(colliding_pairs.size() == colliding_pairs2.size());    
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
