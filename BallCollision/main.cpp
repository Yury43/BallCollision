#include "SFML/Graphics.hpp"
#include "MiddleAverageFilter.h"
#include <iostream>
#include <unordered_set>
#include <functional>
#include <mutex>
#include <assert.h>
#include "math.h"
#include "physicals.h"

constexpr int WINDOW_X = 1024;
constexpr int WINDOW_Y = 768;
//constexpr int WINDOW_Y = 1024;
constexpr int MAX_BALLS = 300;
constexpr int MIN_BALLS = 100;
constexpr float M_PI = 3.1415926;
constexpr int target_framerate = 60;

Math::MiddleAverageFilter<float, 100> fpscounter;

// TODO to prevent fast balls from escaping and for more accurate model in general one should consider positions on the next tick instead of current one


sf::CircleShape ball_as_shape(const Ball& ball)
{
    sf::CircleShape gball;
    gball.setRadius(ball.r);
    gball.setPosition(ball.p.x - ball.r, ball.p.y - ball.r); // consider ball.p to be the center
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
        party1->handle_collision(party2.get());
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

    uint64_t make_id(const std::shared_ptr<Physical> & p1, const std::shared_ptr<Physical> & p2)
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


void phisics_loop()
{
    srand(time(NULL));

    std::vector<std::shared_ptr<Ball>> balls;

    // randomly initialize balls
    for (int i = 0; i < (rand() % (MAX_BALLS - MIN_BALLS) + MIN_BALLS); i++)
        //for (int i = 0; i < 1; i++)
    {
        balls.push_back(std::make_shared<Ball>());

        int r = 5 + rand() % 5;
        balls.back()->r = r;
        balls.back()->p.x = (r + rand()) % (WINDOW_X - r); // make sure balls dont spawn on the edges 
        balls.back()->p.y = (r + rand()) % (WINDOW_Y - r);
        balls.back()->dir.x = (-5.f + (rand() % 10)) / 3.;
        balls.back()->dir.y = (-5.f + (rand() % 10)) / 3.;
        balls.back()->speed = (30.f + rand() % 30) * 1;
    }

    //balls.clear();
    //{
    //    balls.push_back(std::make_shared<Ball>());

    //    balls.back()->r = 20;
    //    balls.back()->p.x = 100;
    //    balls.back()->p.y = 100;
    //    balls.back()->dir.x = -1;
    //    balls.back()->dir.y = -1;
    //    balls.back()->speed = 500;

    //    balls.push_back(std::make_shared<Ball>());
    //    balls.back()->r = 20;
    //    balls.back()->p.x = 300;
    //    balls.back()->p.y = 300;
    //    balls.back()->dir.x = 1;
    //    balls.back()->dir.y = 1;
    //    balls.back()->speed = 500;
    //}


    sf::Clock clock;
    float lastime = clock.restart().asSeconds();



    float max_r = balls.empty() ? 0 : (*std::max_element(balls.begin(), balls.end(), [](const auto& a, const auto& b) { return a->r < b->r; }))->r;

    std::unordered_set<Collision> processed_collisions;

    float total_energy_prev = -1;

    std::cout << "phisics_loop running" << std::endl;

    // calculate target iteration delay and next iteration time
    const auto iteration_delay = std::chrono::milliseconds(1000) / target_framerate;
    auto next_iteration_time = std::chrono::system_clock::now() + iteration_delay;


    while (not stop_flag)
    {
        float current_time = clock.getElapsedTime().asSeconds();
        float deltaTime = current_time - lastime;
        fpscounter.push(1.0f / (current_time - lastime));
        lastime = current_time;

        // Reduce time complexity by placing balls in a grid of bins, only balls in the same bin and its neighbors may interact

        float bin_size = max_r * 4;
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
            // Remove processed collisions parties of which do not touch anymore 
            {
                auto it = processed_collisions.begin();
                while (it != processed_collisions.end())
                {
                    if (it->are_touching())
                    {
                        ++it;
                    }
                    else
                    {
                        it->mark_finished();
                        it = processed_collisions.erase(it);
                    }
                }
            }

            std::vector<Collision> new_collisions;

            // collide with walls 
            for (const auto& ball : balls)
            {
                auto r1 = ball->p;
                float R = ball->r;

                if (r1.x - R < 0)
                {
                    ball->dir.x *= -1;
                    ball->p.x = 0 + R;
                }
                else if (r1.x + R > WINDOW_X)
                {
                    ball->dir.x *= -1;
                    ball->p.x = WINDOW_X - R;
                }

                if (r1.y - R < 0)
                {
                    ball->dir.y *= -1;
                    ball->p.y = 0 + R;
                }
                else if (r1.y + R > WINDOW_Y)
                {
                    ball->dir.y *= -1;
                    ball->p.y = WINDOW_Y - R;
                }
            }


            // Collide with balls 
            for (auto& ball : balls)
            {
                size_t bin_x = find_bin_x(*ball);
                size_t bin_y = find_bin_y(*ball);

                for (int i = 0; i < 2; ++i)
                {
                    for (int j = 0; j < 2; ++j)
                    {
                        if (out_of_range(bin_x + i, bin_y + j)) continue;

                        for (auto& other_ball : bins[bin_x + i][bin_y + j])
                        {
                            if (ball->id == other_ball->id) continue;
                            if (!ball->is_touching(other_ball.get())) continue;

                            new_collisions.push_back(Collision(ball, other_ball));
                        }
                    }
                }
            }

            // Process new collisions and mark them accordingly 
            for (Collision& collision : new_collisions)
            {
                if (processed_collisions.find(collision) == processed_collisions.end())
                {
                    collision.handle();
                    collision.mark_started();
                    processed_collisions.insert(std::move(collision));
                }
            }

            // Apply new reactions 
            for (auto& ball : balls)
            {
                ball->apply_reactions();
            }

            float total_energy = std::accumulate(balls.begin(), balls.end(), 0., [](float sum, const auto& ball) {
                return sum + ball->Energy();
                //return sum + norm(ball->velocity()); 
                });

            if (total_energy_prev > 0)
            {
                float dE = total_energy - total_energy_prev;
                if (dE > 1e-3)
                {
                    std::cout << "dE " << dE << std::endl;
                }
            }
            total_energy_prev = total_energy;
        }

        // Update positions 
        for (auto& ball : balls)
        {
            move_ball(*ball, deltaTime);
        }

        
        // Place shapes to draw in drawing buffer 
        {

            std::vector<sf::CircleShape> gballs;
            gballs.reserve(balls.size());
            for (const auto& ball : balls)
            {
                gballs.push_back(ball_as_shape(*ball));
            }

            {
                std::lock_guard<std::mutex> lock(render_buffer_mutex);
                render_buffer = std::move(gballs);
            }
        }


        // Sleep the remaining time, reserved for iteration and calculate time of next iteration 
        std::this_thread::sleep_until(next_iteration_time);
        next_iteration_time = next_iteration_time + iteration_delay;

    }

    std::cout << "phisics_loop done" << std::endl;
}



int main()
{
    sf::RenderWindow window(sf::VideoMode(WINDOW_X, WINDOW_Y), "ball collision demo");
    window.setFramerateLimit(target_framerate);


    std::thread physics_thread(phisics_loop);

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }
        }

        /// <summary>
        /// TODO: PLACE COLLISION CODE HERE 
        /// объекты создаются в случайном месте на плоскости со случайным вектором скорости, имеют радиус R
        /// Объекты движутся кинетически. Пространство ограниченно границами окна
        /// Напишите обработчик столкновений шаров между собой и краями окна. Как это сделать эффективно?
        /// Массы пропорцианальны площадям кругов, описывающих объекты 
        /// Как можно было-бы улучшить текущую архитектуру кода?
        /// Данный код является макетом, вы можете его модифицировать по своему усмотрению


        // get data to render from phisics thread;
        std::vector<sf::CircleShape> gballs;
        {
            std::lock_guard<std::mutex> lock(render_buffer_mutex);
            gballs = std::move(render_buffer);
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
    physics_thread.join();

    return 0;
}
