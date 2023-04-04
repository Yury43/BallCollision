#include "SFML/Graphics.hpp"
#include "MiddleAverageFilter.h"
#include <iostream>
#include <numeric>
#include <functional>
#include <assert.h>

constexpr int WINDOW_X = 1024;
constexpr int WINDOW_Y = 768;
constexpr int MAX_BALLS = 300;
constexpr int MIN_BALLS = 100;
constexpr float M_PI = 3.14159265358979323846f; // ??

Math::MiddleAverageFilter<float, 100> fpscounter;

template<typename T>
std::ostream& operator<<(std::ostream& os, const sf::Vector2<T>& v)
{
    os << "( " << v.x << " , " << v.y << " ) ";
    return os;
}


struct Ball
{
    sf::Vector2f p = {0, 0};
    sf::Vector2f dir = {0, 0};
    float r = 0;
    float speed = 0;
    sf::Color color = sf::Color::White;
    
    float mass() const 
    {
        return r * r * r; // consider mass to be a function of volume just to make interactions a little bit easier to perceive and comprehend
    }

    sf::Vector2f velocity() const
    {
        return speed * dir;
    }

    sf::Vector2f impulse() const
    {
        return velocity() * mass();
    }

    std::vector<sf::Vector2f> dps = {};
};

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

inline float dot(const sf::Vector2f &a, const sf::Vector2f &b)
{
    return a.x * b.x + a.y * b.y;
}

inline float det(const sf::Vector2f& a, const sf::Vector2f& b)
{
    return a.x * b.y - a.y * b.x;
}

inline float norm(const sf::Vector2f &a)
{
    return std::sqrt(dot(a, a));
}

inline float dist(const sf::Vector2f &a, const sf::Vector2f &b)
{
    return norm(b - a);
}

inline sf::Vector2f normalized(const sf::Vector2f& a)
{
    return a / norm(a);
}

inline float angle(const sf::Vector2f& a, const sf::Vector2f& b)
{
    return std::acos(dot(a, b) / (norm(a) * norm(b)));
}

inline float angle_clockwise(const sf::Vector2f& a, const sf::Vector2f& b)
{
    return std::atan2(det(a, b), dot(a, b));
}



struct Line
{
    sf::Vector2f p1;
    sf::Vector2f p2;
    sf::Vector2f n;

    Line(const sf::Vector2f &_p1, const sf::Vector2f &_p2) : p1(_p1), p2(_p2), n(normalized(p2 - p1))
    {
        
    }
};

inline float dist_to_line(float x1, float y1, float x2, float y2, float x0, float y0)
{
    return 
    std::abs((x2 - x1) * (y1 - y0) - (x1 - x0) * (y2 - y1)) / 
    std::sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}

float dist_to_line(const sf::Vector2f& a, const sf::Vector2f& b, const sf::Vector2f& p)
{
    return dist_to_line(a.x, a.y, b.x, b.y, p.x, p.y);
}


sf::Vector2f project(const Line &l, const sf::Vector2f &p)
{
    const auto & a = l.p1;
    const auto & n = l.n;
    auto d = p - a;
    auto pr = d - (dot(d, n) * n);
    return pr + a;
}

sf::Vector2f project(const sf::Vector2f& a, const sf::Vector2f& b)
{
    return dot(a, b) / norm(b) * normalized(b);
}

float dist(const Line & l, const sf::Vector2f& p)
{
    return norm(project(l, p));
}


bool is_touching(const Line& line, const Ball& ball)
{
    return norm(project(line, ball.p) - ball.p) <= ball.r;
}

bool is_touching(const Ball& b1, const Ball& b2)
{
    return norm(b1.p - b2.p) <= (b1.r + b2.r);
}


void calc_collision(const Line & line, Ball & ball)
{
    auto pos_proj = project(line, ball.p);
    auto ball_to_wall = pos_proj - ball.p;

    sf::Vector2f p0 = ball.impulse();
    sf::Vector2f pn = project(p0, ball_to_wall);
    
    // dont count same collision twice 
    float a = std::abs(angle(ball_to_wall, p0));
    if (a >= M_PI / 2)
    {
        return;
    }

    auto dp = -2.f * pn;
    if (norm(dp) > 1e-6)
    {
        ball.dps.push_back(dp);
    }
}

void calc_collision(Ball& b1, Ball& b2)
{
    auto v1 = b1.velocity();
    auto v2 = b2.velocity();

    auto m1 = b1.mass();
    auto m2 = b2.mass();

    //if (dist(b1.p, b2.p + v2 - v1) >= dist(b1.p, b2.p))
    //{
    //    return;
    //}

    auto c = (b1.p + b2.p) / 2.f;
    auto vc = (v1 + v2) / 2.f;
    float a1 = std::abs(angle(c - b1.p, v1 - vc));
    float a2 = std::abs(angle(c - b2.p, v2 - vc));

    if (std::max(a1, a2) >= M_PI / 2)
    {
        return;
    }
    
    auto dv1 = (m2 * v2 * 2.f + v1 * (m1 - m2)) / (m1 + m2) - v1;
    sf::Vector2f dv2 = (m1 * v1 * 2.f + v2 * (m2 - m1)) / (m1 + m2) - v2;

    if (norm(dv1) > 1e-6)
    {
        b1.dps.push_back(dv1 * m1);
    }

    if (norm(dv2) > 1e-6)
    {
        b2.dps.push_back(dv2 * m2);
    }
}

template<typename T>
sf::Vector2<T> average(const std::vector<sf::Vector2<T>>& items)
{
    return std::accumulate(items.begin(), items.end(), sf::Vector2<T>(0, 0)) / static_cast<T>(items.size());
}

void apply_collision(Ball & ball)
{
    auto p0 = ball.impulse();

    sf::Vector2f dp = average(ball.dps);
    ball.dps.clear();

    auto p2 = p0 + dp;

    auto adp = std::abs(norm(p0) - norm(p2));

    //assert(adp < 1e-2); // impulse conservation "law"

    auto v = p2 / ball.mass();
    ball.speed = norm(v);
    ball.dir = normalized(v);
}


int main()
{
    sf::RenderWindow window(sf::VideoMode(WINDOW_X, WINDOW_Y), "ball collision demo");
    srand(time(NULL));

    std::vector<Ball> balls;

    // randomly initialize balls
    for (int i = 0; i < (rand() % (MAX_BALLS - MIN_BALLS) + MIN_BALLS); i++)
    //for (int i = 0; i < 1; i++)
    {
        Ball newBall;
        newBall.p.x = rand() % WINDOW_X;
        newBall.p.y = rand() % WINDOW_Y;
        newBall.dir.x = (-5 + (rand() % 10)) / 3.;
        newBall.dir.y = (-5 + (rand() % 10)) / 3.;
        newBall.r = 5 + rand() % 5;
        newBall.speed = (30 + rand() % 30) * 1;

        balls.push_back(newBall);
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

        {
            for (int i = 0; i < balls.size(); ++i)
            {
                for (int j = 0; j < walls.size(); ++j)
                {
                    if (is_touching(walls[j], balls[i]))
                    {
                        calc_collision(walls[j], balls[i]);
                    }
                }

                for (int j = i + 1; j < balls.size(); ++j) // n^2 
                {
                    if (is_touching(balls[j], balls[i]))
                    {
                        calc_collision(balls[j], balls[i]);
                    }
                }
            }

            for (auto& ball : balls)
            {
                if (!ball.dps.empty())
                {
                    apply_collision(ball);
                    ball.color = sf::Color::Red;
                }
                else
                {
                    ball.color = sf::Color::White;
                }
            }
        }

        for (auto& ball : balls)
        {
            move_ball(ball, deltaTime);
        }

        window.clear();

        for (const auto & ball : balls)
        {
            draw_ball(window, ball);
        }


        //draw_fps(window, fpscounter.getAverage());
        window.display();
    }
    return 0;
}
