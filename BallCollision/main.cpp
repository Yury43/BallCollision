#include "SFML/Graphics.hpp"
#include "MiddleAverageFilter.h"
#include <iostream>
#include <numeric>
#include <unordered_set>
#include <functional>
#include <assert.h>
#include "math.h"
#include "Physicals.h"

constexpr int WINDOW_X = 1024;
constexpr int WINDOW_Y = 768;
constexpr int MAX_BALLS = 300;
constexpr int MIN_BALLS = 100;
constexpr float M_PI = 3.1415926;

Math::MiddleAverageFilter<float, 100> fpscounter;

// TODO to prevent fast balls from escaping and for more accurate model in general one should consider positions on the next tick instead of current one

template<typename T>
std::ostream& operator<<(std::ostream& os, const sf::Vector2<T>& v)
{
    os << "( " << v.x << " , " << v.y << " ) ";
    return os;
}


void draw_ball(sf::RenderWindow& window, const Ball& ball)
{
    sf::CircleShape gball;
    gball.setRadius(ball.r);
    gball.setPosition(ball.p.x - ball.r, ball.p.y - ball.r); // consider ball.p to be the center
    gball.setFillColor(ball.color);
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


template<typename T>
sf::Vector2<T> average(const std::vector<sf::Vector2<T>>& items)
{
    return std::accumulate(items.begin(), items.end(), sf::Vector2<T>(0, 0)) / static_cast<T>(items.size());
}

void apply_reactions(Ball & ball)
{
    auto p0 = ball.impulse();

    sf::Vector2f dp = average(ball.reactions);
    ball.reactions.clear();

    auto p2 = p0 + dp;

    auto adp = std::abs(norm(p0) - norm(p2));

    //assert(adp < 1e-2); // impulse conservation "law"

    auto v = p2 / ball.mass();
    ball.speed = norm(v);
    ball.dir = normalized(v);
}

class Collision
    {

public:

    const uint64_t cid;

    Collision(Physical* p1, Physical* p2) : party1(p1), party2(p2), cid(make_id(p1, p2))
    {
        
    }

    bool are_touching() const 
    {
        return party1->is_touching(party2);
    }

    void handle()
    {
        {
            auto p1 = dynamic_cast<Ball*>(party1);
            auto p2 = dynamic_cast<Ball*>(party2);
            if (p1 != nullptr and p2 != nullptr)
            {
                handle(*p1, *p2);
                return;
            }
        }
        {
            auto p1 = dynamic_cast<Line*>(party1);
            auto p2 = dynamic_cast<Ball*>(party2);
            if (p1 != nullptr and p2 != nullptr)
            {
                handle(*p1, *p2);
                return;
            }
        }
        {
            auto p1 = dynamic_cast<Ball*>(party1);
            auto p2 = dynamic_cast<Line*>(party2);
            if (p1 != nullptr and p2 != nullptr)
            {
                handle(*p2, *p1);
                return;
            }
        }
        throw std::logic_error("Not implemented");
    }

    void mark_start() const
    {
        auto p1 = dynamic_cast<Ball*>(party1);
        auto p2 = dynamic_cast<Ball*>(party2);
        if (p1 != nullptr)
        {
            p1->color = sf::Color::Red;
        }
        if (p2 != nullptr)
        {
            p2->color = sf::Color::Red;
        }
    }

    void mark_finish() const
    {
        auto p1 = dynamic_cast<Ball*>(party1);
        auto p2 = dynamic_cast<Ball*>(party2);
        if (p1 != nullptr)
        {
            p1->color = sf::Color::White;
        }
        if (p2 != nullptr)
        {
            p2->color = sf::Color::White;
        }
    }

private:
    Physical* party1 = nullptr;
    Physical* party2 = nullptr;

    uint64_t make_id(Physical* p1, Physical* p2)
    {
        int high = p1->id;
        int low = p2->id;
        return (((uint64_t)high) << 32) | ((uint64_t)low);
    }

    void handle(const Line& line, Ball& ball)
    {
        auto pos_proj = project(line, ball.p);
        auto ball_to_wall = pos_proj - ball.p;

        sf::Vector2f p0 = ball.impulse();
        sf::Vector2f pn = project(p0, ball_to_wall);

        auto dp = -2.f * pn;
        if (norm(dp) > 1e-6)
        {
            ball.reactions.push_back(dp);
        }
    }

    void handle(Ball& b1, Ball& b2)
    {
        auto v1 = b1.velocity();
        auto v2 = b2.velocity();

        auto m1 = b1.mass();
        auto m2 = b2.mass();


        auto dv1 = (m2 * v2 * 2.f + v1 * (m1 - m2)) / (m1 + m2) - v1;
        sf::Vector2f dv2 = (m1 * v1 * 2.f + v2 * (m2 - m1)) / (m1 + m2) - v2;

        if (norm(dv1) > 1e-6)
        {
            b1.reactions.push_back(dv1 * m1);
        }

        if (norm(dv2) > 1e-6)
        {
            b2.reactions.push_back(dv2 * m2);
        }
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

int main()
{
    sf::RenderWindow window(sf::VideoMode(WINDOW_X, WINDOW_Y), "ball collision demo");
    srand(time(NULL));

    std::vector<std::shared_ptr<Ball>> balls;

    // randomly initialize balls
    for (int i = 0; i < (rand() % (MAX_BALLS - MIN_BALLS) + MIN_BALLS); i++)
    //for (int i = 0; i < 1; i++)
    {
        balls.push_back(std::make_shared<Ball>());

        balls.back()->p.x = rand() % WINDOW_X;
        balls.back()->p.y = rand() % WINDOW_Y;
        balls.back()->dir.x = (-5 + (rand() % 10)) / 3.;
        balls.back()->dir.y = (-5 + (rand() % 10)) / 3.;
        balls.back()->r = 5 + rand() % 5;
        balls.back()->speed = (30 + rand() % 30) * 1;
    }
     
    window.setFramerateLimit(60);

    sf::Clock clock;
    float lastime = clock.restart().asSeconds();

    sf::Vector2f tl(0, 0);
    sf::Vector2f tr(WINDOW_X, 0);
    sf::Vector2f br(WINDOW_X, WINDOW_Y);
    sf::Vector2f bl(0, WINDOW_Y);

    std::vector<Line> walls = 
    {
        Line(tl, tr),
        Line(tr, br),
        Line(br, bl),
        Line(bl, tl),
    };

    float max_r = balls.empty() ? 0 : (*std::max_element(balls.begin(), balls.end(), [](const auto& a, const auto& b) { return a->r < b->r; }))->r;

    std::unordered_set<Collision> processed_collisions;

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

        float current_time = clock.getElapsedTime().asSeconds();
        float deltaTime = current_time - lastime;
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

        // n log (n) complexity on average, because candidates are considered only in a limited (max_r * 2) X window to right 
        std::sort(balls.begin(), balls.end(), [](const auto & a, const auto & b) { return a->p.x < b->p.x; });

        {
            // remove processed collisions parties of which do not touch anymore 
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
                        it->mark_finish();
                        it = processed_collisions.erase(it);
                    }
                }
            }

            std::vector<Collision> new_collisions;

            for (int i = 0; i < balls.size(); ++i)
            {
                for (int j = 0; j < walls.size(); ++j)
                {
                    if (are_touching(walls[j], *balls[i]))
                    {
                        new_collisions.push_back(Collision((Physical*) balls[i].get(), (Physical*) &walls[j]));
                    }
                }

                for (int j = i + 1; j < balls.size(); ++j) 
                {
                    if (balls[j]->p.x - balls[i]->p.x > max_r + balls[i]->r)
                    {
                        break; // skip balls that are surely out of reach
                    }

                    if (are_touching(*balls[j], *balls[i]))
                    {
                        new_collisions.push_back(Collision((Physical*) balls[i].get(), (Physical*) balls[j].get()));
                    }
                }
            }

            // process new collisions and mark them accordingly 
            for (Collision& collision : new_collisions)
            {
                if (processed_collisions.find(collision) == processed_collisions.end())
                {
                    collision.handle();
                    collision.mark_start();
                    processed_collisions.insert(std::move(collision));
                }
            }

            // apply new reactions 
            for (auto& ball : balls)
            {
                if (!ball->reactions.empty())
                {
                    apply_reactions(*ball);
                }
            }
        }

        for (auto& ball : balls)
        {
            move_ball(*ball, deltaTime);
        }

        window.clear();

        for (const auto & ball : balls)
        {
            draw_ball(window, *ball);
        }


        //draw_fps(window, fpscounter.getAverage());
        window.display();
    }
    return 0;
}
