#pragma once

#include <vector>
#include <memory>
#include <random>
#include "physicals.h"
#include "constants.h"
#include "randgen.h"


// randomly initialize balls
inline void init_random(std::vector<std::shared_ptr<Ball>>& balls)
{
    std::mt19937 rand_gen = RandGen::get();
    
    for (int i = 0; i < (rand_gen() % (MAX_BALLS - MIN_BALLS) + MIN_BALLS); i++)
        //for (int i = 0; i < 1; i++)
    {
        balls.push_back(std::make_shared<Ball>());

        int r = 5 + rand_gen() % 5;
        balls.back()->R = r;
        balls.back()->p.x = (r + rand_gen()) % (WINDOW_X - r); // make sure balls dont spawn on the edges 
        balls.back()->p.y = (r + rand_gen()) % (WINDOW_Y - r);
        balls.back()->dir.x = (-5.f + (rand_gen() % 10)) / 3.f;
        balls.back()->dir.y = (-5.f + (rand_gen() % 10)) / 3.f;
        balls.back()->speed = (30 + rand_gen() % 30) * 1;
    }
}

inline void init_corner_bounce(std::vector<std::shared_ptr<Ball>>& balls)
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

inline void init_chain(std::vector<std::shared_ptr<Ball>>& balls)
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

inline void init_snooker(std::vector<std::shared_ptr<Ball>>& balls)
{
    float R = 20;

    Ball ball;

    // 0
    ball.R = R;
    ball.p.x = 1.f * WINDOW_X - R - 1;
    ball.p.y = 1.f * WINDOW_Y / 2;
    ball.dir.x = -1;
    ball.dir.y = 0;
    ball.speed = 500;
    balls.push_back(std::make_shared<Ball>(ball));

    //1 
    ball.p.x = 1.f * WINDOW_X / 2;
    ball.p.y = 1.f * WINDOW_Y / 2;
    ball.dir.x = 0;
    ball.speed = 0;
    balls.push_back(std::make_shared<Ball>(ball));

    for (int j = 2; j <= 5; ++j)
    {
        float sign = j % 2 == 0 ? 1 : -1;

        ball.p.x -= R * 2 * std::cos(M_PI / 6);
        ball.p.y += R * 3 * sign;

        for (int i = 0; i < j; ++i)
        {
            ball.p.y -= R * 2 * sign;
            balls.push_back(std::make_shared<Ball>(ball));
        }
    }

    std::mt19937 rand_gen = RandGen::get();
    
    for (auto& b : balls)
    {
        b->R += 0.001f * (4 - rand_gen() % 5);
    }

}

inline void init_angled1(std::vector<std::shared_ptr<Ball>>& balls)
{
    float R = 20;

    Ball ball;

    ball.R = R;
    ball.p.x = 1.f * WINDOW_X - R - 1;
    ball.p.y = 1.f * WINDOW_Y / 2;
    ball.dir.x = -1;
    ball.dir.y = 0;
    ball.speed = 250;
    balls.push_back(std::make_shared<Ball>(ball));

    ball.p.x = 1.f * WINDOW_X / 2;
    ball.p.y = 1.f * WINDOW_Y / 2;
    ball.p.y += R;
    ball.dir.x = 0;
    ball.speed = 0;
    balls.push_back(std::make_shared<Ball>(ball));
}

inline void init_angled2(std::vector<std::shared_ptr<Ball>>& balls)
{
    float R = 20;

    Ball ball;

    ball.R = R;
    ball.p.x = 1.f * WINDOW_X - R - 1;
    ball.p.y = 1.f * WINDOW_Y / 2;
    ball.dir.x = -1;
    ball.dir.y = 0;
    ball.speed = 250;
    balls.push_back(std::make_shared<Ball>(ball));

    ball.p.x = 1.f * WINDOW_X / 2;
    ball.p.y = 1.f * WINDOW_Y / 2;
    ball.p.y -= R * 1.5f;
    ball.dir.x = 0;
    ball.speed = 0;
    balls.push_back(std::make_shared<Ball>(ball));
}