#include "SFML/Graphics.hpp"
#include "MiddleAverageFilter.h"
#include <iostream>
#include <unordered_set>
#include <unordered_map>
#include <functional>
#include <mutex>
#include <assert.h>
#include "math.h"
#include "physicals.h"
#include "disjoined_set_union.h"
#include <iomanip>


constexpr int WINDOW_X = 1024;
constexpr int WINDOW_Y = 768;
//constexpr int WINDOW_Y = 1024;
constexpr int MAX_BALLS = 300;
constexpr int MIN_BALLS = 100;
constexpr float M_PI = 3.1415926;
constexpr int TARGET_FRAMERATE = 60;
//constexpr bool ASYNC_PHYS = false; // run simulation is the same thread as the window 
constexpr bool ASYNC_PHYS = true; // run simulation in a thread separate from the window 
//constexpr bool MOVE_BEFORE_COLLISION = false; // Update positions after searching for collisions to render state preceding collision of next iteration 
constexpr bool MOVE_BEFORE_COLLISION = true; // Update positions before searching for collisions to account for current positions results in a more accurate sim 


// TODO to prevent fast balls from escaping and for more accurate model in general one should consider positions on the next tick instead of current one


sf::CircleShape ball_as_shape(const Ball& ball)
{
    sf::CircleShape gball;
    gball.setRadius(ball.R);
    gball.setPosition(ball.p.x - ball.R, ball.p.y - ball.R); // consider ball.p to be the center
    gball.setFillColor(ball.color);
    return gball;
}

void draw_ball(sf::RenderWindow& window, const Ball& ball)
{
    sf::CircleShape gball = ball_as_shape(ball);
    window.draw(gball);
}

void move_ball(Ball& ball, float deltaTime)
{
    float dx = ball.dir.x * ball.speed * deltaTime;
    float dy = ball.dir.y * ball.speed * deltaTime;
    ball.p.x += dx;
    ball.p.y += dy;
}

void draw_fps(sf::RenderWindow& window, float fps)
{
    char c[32];
    snprintf(c, 32, "FPS: %f", fps);
    sf::String str(c);
    window.setTitle(str);
}


class Collision
    {

public:

    const uint64_t cid;

    Collision(
        const std::shared_ptr<Physical> & p1,
        const std::shared_ptr<Physical> & p2)
        : party1(p1), party2(p2), cid(make_id(p1, p2))
    {
        
    }

    bool are_touching() const 
    {
        return party1->is_touching(party2.get());
    }

    void handle()
    {
        return party1->handle_collision(party2.get());
    }

    void mark_started() const
    {
        party1->mark_colliding(true);
        party2->mark_colliding(true);
    }

    void mark_finished() const
    {
        party1->mark_colliding(false);
        party2->mark_colliding(false);
    }

private:
    std::shared_ptr<Physical> party1;
    std::shared_ptr<Physical> party2;

    static uint64_t make_id(const std::shared_ptr<Physical> & p1, const std::shared_ptr<Physical> & p2)
    {
        int high = p1->id;
        int low = p2->id;

        if (high > low) // make collisions of same objects identical 
        {
            std::swap(low, high);
        }

        return (((uint64_t)high) << 32) | ((uint64_t)low);
    }
};

namespace std 
{
    template <>
    struct hash<Collision>
    {
        std::size_t operator()(const Collision& c) const
        {
            return c.cid;
        }
    };
}

bool operator == (const Collision & left, const Collision& right)
{
    return left.cid == right.cid;
}


std::atomic_bool stop_flag = false;
std::vector<sf::CircleShape> render_buffer;
std::mutex render_buffer_mutex;

class Simulation
{
    std::vector<std::shared_ptr<Ball>> balls;
    std::unordered_map<int, std::shared_ptr<Ball>> balls_by_id;
    float max_r;
    double total_energy_prev = -1;
    double initial_enery = -1;

    // randomly initialize balls
    void init_random(std::vector<std::shared_ptr<Ball>>& balls)
    {
        srand(time(NULL));

        for (int i = 0; i < (rand() % (MAX_BALLS - MIN_BALLS) + MIN_BALLS); i++)
            //for (int i = 0; i < 1; i++)
        {
            balls.push_back(std::make_shared<Ball>());

            int r = 5 + rand() % 5;
            balls.back()->R = r;
            balls.back()->p.x = (r + rand()) % (WINDOW_X - r); // make sure balls dont spawn on the edges 
            balls.back()->p.y = (r + rand()) % (WINDOW_Y - r);
            balls.back()->dir.x = (-5.f + (rand() % 10)) / 3.;
            balls.back()->dir.y = (-5.f + (rand() % 10)) / 3.;
            balls.back()->speed = (30.f + rand() % 30) * 1;
        }
    }

    void init_corner_bounce(std::vector<std::shared_ptr<Ball>>& balls)
    {
        Ball ball;

        ball.R = 20;
        ball.p.x = 100;
        ball.p.y = 100;
        ball.dir.x = -1;
        ball.dir.y = -1;
        ball.speed = 500;

        balls.push_back(std::make_shared<Ball>(ball));

        ball.p.x = 300;
        ball.p.y = 300;
        ball.dir.x = 1;
        ball.dir.y = 1;

        balls.push_back(std::make_shared<Ball>(ball));
    }

    void init_chain(std::vector<std::shared_ptr<Ball>>& balls)
    {
        float R = 20;

        Ball ball;

        ball.R = R;
        ball.p.x = 100;
        ball.p.y = 100;
        ball.dir.x = -1;
        ball.dir.y = 0;
        ball.speed = 500;
        balls.push_back(std::make_shared<Ball>(ball));

        ball.p.x += 2 * R;
        ball.dir.x = 0;
        ball.speed = 0;
        balls.push_back(std::make_shared<Ball>(ball));

        ball.p.x += 2 * R;
        balls.push_back(std::make_shared<Ball>(ball));

        ball.p.x += 2 * R;
        balls.push_back(std::make_shared<Ball>(ball));
    }

    void init_snooker(std::vector<std::shared_ptr<Ball>>& balls)
    {
        float R = 20;

        Ball ball;

        // 0
        ball.R = R;
        ball.p.x = WINDOW_X - R - 1;
        ball.p.y = WINDOW_Y / 2;
        ball.dir.x = -1;
        ball.dir.y = 0;
        ball.speed = 500;
        balls.push_back(std::make_shared<Ball>(ball));

        //1 
        ball.p.x = WINDOW_X / 2;
        ball.p.y = WINDOW_Y / 2;
        ball.dir.x = 0;
        ball.speed = 0;
        balls.push_back(std::make_shared<Ball>(ball));

        for (int j = 2; j <= 5; ++j)
        {
            int sign = j % 2 == 0 ? 1 : -1;

            ball.p.x -= R * 2 * std::cos(M_PI / 6);
            ball.p.y += R * 3 * sign;

            for (int i = 0; i < j; ++i)
            {
                ball.p.y -= R * 2 * sign;
                balls.push_back(std::make_shared<Ball>(ball));
            }
        }

        for (auto& ball : balls)
        {
            ball->R += 0.001 * (4 - rand() % 5);
        }

    }

    void init_angled1(std::vector<std::shared_ptr<Ball>>& balls)
    {
        float R = 20;

        Ball ball;

        ball.R = R;
        ball.p.x = WINDOW_X - R - 1;
        ball.p.y = WINDOW_Y / 2;
        ball.dir.x = -1;
        ball.dir.y = 0;
        ball.speed = 250;
        balls.push_back(std::make_shared<Ball>(ball));

        ball.p.x = WINDOW_X / 2;
        ball.p.y = WINDOW_Y / 2;
        ball.p.y += R;
        ball.dir.x = 0;
        ball.speed = 0;
        balls.push_back(std::make_shared<Ball>(ball));
    }

    void init_angled2(std::vector<std::shared_ptr<Ball>>& balls)
    {
        float R = 20;

        Ball ball;

        ball.R = R;
        ball.p.x = WINDOW_X - R - 1;
        ball.p.y = WINDOW_Y / 2;
        ball.dir.x = -1;
        ball.dir.y = 0;
        ball.speed = 250;
        balls.push_back(std::make_shared<Ball>(ball));

        ball.p.x = WINDOW_X / 2;
        ball.p.y = WINDOW_Y / 2;
        ball.p.y -= R * 1.5;
        ball.dir.x = 0;
        ball.speed = 0;
        balls.push_back(std::make_shared<Ball>(ball));
    }


    void move_balls(float deltaTime)
    {
        for (auto& ball : balls)
        {
            move_ball(*ball, deltaTime);
        }
    }

public:
    void init()
    {
        init_random(balls);
        //init_corner_bounce(balls);
        //init_chain(balls);
        //init_snooker(balls);
        //init_angled1(balls);
        //init_angled2(balls);
        
        for (const auto& ball : balls)
        {
            balls_by_id[ball->id] = ball;
        }
        
        max_r = balls.empty() ? 0 : (*std::max_element(balls.begin(), balls.end(), [](const auto& a, const auto& b) { return a->R < b->R; }))->R;
    }

    std::vector<sf::CircleShape> run_iteration(float deltaTime)
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
                if (ball->test_wall_collision(0, 0, WINDOW_X, WINDOW_Y))
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

            for (const std::pair<int, std::vector<int>>& collision_set : colliding_sets.SetsByParent())
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
                    return sum + ball->Energy();
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

    void phisics_loop()
    {
        sf::Clock clock;
        float lastime = clock.restart().asSeconds();
        float speed_up = 1;
        // calculate target iteration delay and next iteration time
        const auto iteration_delay = std::chrono::milliseconds(1000) / TARGET_FRAMERATE / speed_up;
        auto next_iteration_time = std::chrono::system_clock::now() + iteration_delay;
        std::cout << "phisics_loop running" << std::endl;

        while (not stop_flag)
        {
            float current_time = clock.getElapsedTime().asSeconds();
            float deltaTime = (current_time - lastime) * speed_up;
            lastime = current_time;

            auto gballs = run_iteration(deltaTime);

            // Place shapes to draw in drawing buffer 
            {
                std::lock_guard<std::mutex> lock(render_buffer_mutex);
                render_buffer = std::move(gballs);
            }

            // Sleep the remaining time, reserved for iteration and calculate time of next iteration 
            std::this_thread::sleep_until(next_iteration_time);
            next_iteration_time = next_iteration_time + iteration_delay;

        }

        std::cout << "phisics_loop done" << std::endl;
    }
};




int main()
{
    sf::RenderWindow window(sf::VideoMode(WINDOW_X, WINDOW_Y), "ball collision demo");
    window.setFramerateLimit(TARGET_FRAMERATE);

    Simulation sim;
    sim.init();

    std::thread physics_thread;

    if (ASYNC_PHYS)
    {
        physics_thread = std::thread(([&sim] {sim.phisics_loop(); }));
    }
    
    sf::Clock clock;
    float lastime = clock.restart().asSeconds();
    Math::MiddleAverageFilter<float, 100> fpscounter;

    while (window.isOpen())
    {
        sf::Event event;
        float poll_start = clock.getElapsedTime().asSeconds();
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }
        }
        float poll_end = clock.getElapsedTime().asSeconds();

        float current_time = clock.getElapsedTime().asSeconds();
        float deltaTime = (current_time - lastime);
        float simDeltaTime = deltaTime - (poll_end - poll_start);
        fpscounter.push(1.0f / (current_time - lastime));
        lastime = current_time;

        /// <summary>
        /// TODO: PLACE COLLISION CODE HERE 
        /// объекты создаются в случайном месте на плоскости со случайным вектором скорости, имеют радиус R
        /// Объекты движутся кинетически. Пространство ограниченно границами окна
        /// Напишите обработчик столкновений шаров между собой и краями окна. Как это сделать эффективно?
        /// Массы пропорцианальны площадям кругов, описывающих объекты 
        /// Как можно было-бы улучшить текущую архитектуру кода?
        /// Данный код является макетом, вы можете его модифицировать по своему усмотрению

        std::vector<sf::CircleShape> gballs;

        if (ASYNC_PHYS)
        {
            // get data to render from phisics thread;
            {
                std::lock_guard<std::mutex> lock(render_buffer_mutex);
                gballs = std::move(render_buffer);
            }
        }
        else
        {
            gballs = sim.run_iteration(simDeltaTime);
        }


        if (!gballs.empty())
        {
            window.clear();
            for (const auto& gball : gballs)
            {
                window.draw(gball);
            }
        }

        draw_fps(window, fpscounter.getAverage());
        window.display();
    }

    stop_flag = true;
    
    if (physics_thread.joinable())
    {
        physics_thread.join();
    }

    return 0;
}
