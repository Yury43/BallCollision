#include "engine.h"
#include "scenarios.h"
#include "disjoined_set_union.h"
#include "collision.h"
#include <iomanip>

sf::CircleShape ball_as_shape(const Ball& ball)
{
    sf::CircleShape gball;
    gball.setRadius(ball.R);
    gball.setPosition(ball.p.x - ball.R, ball.p.y - ball.R); // consider ball.p to be the center
    gball.setFillColor(ball.color);
    return gball;
}

Engine::Engine()
{
    //init_random(balls);
    //init_corner_bounce(balls);
    //init_chain(balls);
    init_snooker(balls);
    //init_angled1(balls);
    //init_angled2(balls);

    for (const auto& ball : balls)
    {
        balls_by_id[ball->id] = ball;
    }

    max_r = balls.empty() ? 0 : (*std::max_element(balls.begin(), balls.end(), [](const auto& a, const auto& b) { return a->R < b->R; }))->R;
}

std::vector<sf::CircleShape> Engine::run_iteration(float deltaTime)
{
    if (MOVE_BEFORE_COLLISION)
    {
        move_balls(deltaTime);
    }

    // Reduce time complexity by placing balls in a grid of bins, only balls in the same bin and its neighbors may interact

    float bin_size = max_r * 2;
    int n_bins_x = std::ceil(WINDOW_X / bin_size) + 2;
    int n_bins_y = std::ceil(WINDOW_Y / bin_size) + 2;

    using BallBin = std::vector<std::shared_ptr<Ball>>;
    std::vector<std::vector<BallBin>> bins(n_bins_x, std::vector<BallBin>(n_bins_y));

    auto find_bin_x = [bin_size, n_bins_x](const Ball& b) {return static_cast<int>(std::round(b.p.x / bin_size)) + 1; };
    auto find_bin_y = [bin_size, n_bins_y](const Ball& b) {return static_cast<int>(std::round(b.p.y / bin_size)) + 1; };
    auto out_of_range = [n_bins_x, n_bins_y](int bin_x, int bin_y) {return bin_x < 0 or bin_x >= n_bins_x or bin_y < 0 or bin_y >= n_bins_y; };

    // Place balls in bins, delete those that escaped 
    {
        auto ball_it = balls.begin();
        while (ball_it != balls.end())
        {
            int bin_x = find_bin_x(**ball_it);
            int bin_y = find_bin_y(**ball_it);

            if (out_of_range(bin_x, bin_y))
            {
                std::cout << "Ball " << (*ball_it)->id << " " << (*ball_it)->p << " escaped to [" << bin_x << ", " << bin_y << "] !" << std::endl;
                ball_it = balls.erase(ball_it);
            }
            else
            {
                bins[bin_x][bin_y].push_back(*ball_it);
                ++ball_it;
            }
        }
    }

    {
        std::vector<std::pair<int, int>> colliding_pairs;

        for (const auto& ball : balls)
        {
            // Test and handle collision with walls 
            // Wall collisions have higher priority 
            if (ball->test_and_handle_wall_collision(0, 0, WINDOW_X, WINDOW_Y))
            {
                continue;
            }

            // Test collision with other balls in bins 

            size_t bin_x = find_bin_x(*ball);
            size_t bin_y = find_bin_y(*ball);

            for (int i = 0; i < 2; ++i)
            {
                for (int j = 0; j < 2; ++j)
                {
                    if (out_of_range(bin_x + i, bin_y + j))
                        continue;

                    for (auto& other_ball : bins[bin_x + i][bin_y + j])
                    {
                        if (ball->id == other_ball->id)
                            continue;

                        if (!ball->is_touching(other_ball.get()))
                            continue;

                        colliding_pairs.push_back({ ball->id, other_ball->id });
                    }
                }
            }
        }

        // Find collisions involving same balls 

        DisjoinedSetUnion colliding_sets(colliding_pairs.size());

        for (int i = 0; i < colliding_pairs.size(); ++i)
        {
            for (int j = i + 1; j < colliding_pairs.size(); ++j)
            {
                if (colliding_pairs[i].first == colliding_pairs[j].first ||
                    colliding_pairs[i].first == colliding_pairs[j].second ||
                    colliding_pairs[i].second == colliding_pairs[j].first ||
                    colliding_pairs[i].second == colliding_pairs[j].second)
                {
                    colliding_sets.Join(i, j);
                }
            }
        }

        // If collision involves more than 2 balls, chose single pair randomly, leave others for later 

        std::vector<Collision> new_collisions;

        for (std::pair<const int, std::vector<int>>& collision_set : colliding_sets.SetsByParent())
        {
            int selected_pair_id = collision_set.second[rand() % collision_set.second.size()];
            std::pair<int, int> selected_pair = colliding_pairs[selected_pair_id];

            new_collisions.push_back(Collision(
                balls_by_id[selected_pair.first],
                balls_by_id[selected_pair.second]
            ));
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


        if (false)
        {
            // Calculate total kinetic energy change to control accuracy of simulation 

            float total_energy = std::accumulate(balls.begin(), balls.end(), 0., [](float sum, const auto& ball) {
                return sum + ball->energy();
                //return sum + norm(ball->velocity()); 
                });

            if (initial_enery < 0)
            {
                initial_enery = total_energy;
            }

            if (total_energy_prev > 0)
            {
                float dE = total_energy - total_energy_prev;
                float dEp = dE / total_energy_prev * 100;
                if (std::abs(dE) > 1e-1)
                {
                    //std::cout << std::showpos << std::fixed << std::setprecision(0) <<  << std::endl;
                    std::cout << std::fixed << std::setprecision(0)
                        << "dE " << std::setw(6) << std::showpos << dEp << " %"
                        << "\ttot E " << std::setw(6) << std::noshowpos << total_energy / initial_enery * 100 << " % "
                        << std::endl;
                }
            }
            total_energy_prev = total_energy;
        }
    }

    if (not MOVE_BEFORE_COLLISION)
    {
        move_balls(deltaTime);
    }

    std::vector<sf::CircleShape> gballs;
    gballs.reserve(balls.size());
    for (const auto& ball : balls)
    {
        gballs.push_back(ball_as_shape(*ball));
    }

    return gballs;
}

void Engine::move_ball(Ball& ball, float deltaTime)
{
    float dx = ball.dir.x * ball.speed * deltaTime;
    float dy = ball.dir.y * ball.speed * deltaTime;
    ball.p.x += dx;
    ball.p.y += dy;
}

void Engine::move_balls(float deltaTime)
{
    for (auto& ball : balls)
    {
        move_ball(*ball, deltaTime);
    }
}
