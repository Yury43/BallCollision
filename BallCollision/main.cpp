#include "SFML/Graphics.hpp"
#include "MiddleAverageFilter.h"
#include <iostream>
#include <assert.h>

constexpr int WINDOW_X = 1024;
constexpr int WINDOW_Y = 768;
constexpr int MAX_BALLS = 300;
constexpr int MIN_BALLS = 100;
constexpr float M_PI = 3.14159265358979323846f;

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
    
    sf::Vector2f velocity() const
    {
        return speed * dir;
    }

    sf::Vector2f impulse() const
    {
        return velocity() * r;
    }
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
    //std::string string(c);
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


bool calc_collision(const Line & line, const Ball & ball, sf::Vector2f & dp)
{
    auto pos_proj = project(line, ball.p);
    auto ball_to_wall = pos_proj - ball.p;
    float d = norm(ball_to_wall);
    //std::cout << d << "\t";

    if (d > ball.r + 1e-3)
    {
        return false;
    }

    sf::Vector2f p0 = ball.impulse();
    sf::Vector2f pn = project(p0, ball_to_wall);
    
    // dont count same collision twice 
    float a = std::abs(angle(ball_to_wall, p0));
    if (a >= M_PI / 2)
    {
        return false;
    }

    dp += -2.f * pn;
    
    //std::cout << a << "\t" << normalized(p0) << "\t" << normalized(ball_to_wall)  << "\t" << dp << std::endl;;

    return true;
}

void apply_collision(Ball & ball, const sf::Vector2f & dp)
{
    auto p0 = ball.impulse();
    auto p2 = p0 + dp;

    auto adp = std::abs(norm(p0) - norm(p2));

    assert(adp < 1e-2); // impulse conservation "law"

    auto v = p2 / ball.r;
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

    //{
    //    Ball ball;
    //    ball.r = 10;
    //    ball.p = tr;
    //    balls.push_back(ball);
    //}

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

        std::vector<Ball> projections(walls.size());

        {
            for (Ball& ball : balls)
            {
                bool collides = false;
                sf::Vector2f dp(0, 0);
                //std::cout << std::endl;
                for (int i = 0; i < walls.size(); ++i)
                {
                    if (i != 2)
                    {
                        //continue;
                    }

                    collides |= is_touching(walls[i], ball);
                    calc_collision(walls[i], ball, dp);
                    
                    projections[i] = ball;
                    projections[i].p = project(walls[i], ball.p);
                    projections[i].color = sf::Color::Green;
                    //std::cout << i << " " << projections[i].p << std::endl;
                }
                
                //std::cout << dp << std::endl;

                ball.color = collides ? sf::Color::Red : sf::Color::White;
                if (norm(dp) > 1e-6)
                {
                    //std::cout << dp << std::endl;
                    apply_collision(ball, dp);
                }
            }
        }

        for (auto& ball : balls)
        {
            move_ball(ball, deltaTime);
        }

        window.clear();

        //for (const auto& ball : projections)
        //{
        //    draw_ball(window, ball);
        //}

        for (const auto & ball : balls)
        {
            draw_ball(window, ball);
        }


        //draw_fps(window, fpscounter.getAverage());
        window.display();
    }
    return 0;
}
