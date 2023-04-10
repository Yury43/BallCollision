#pragma once

#include <vector>
#include <memory>
#include <random>
#include "physicals.h"
#include "constants.h"
#include "randgen.h"
#include "ball.h"

// Different testing scenarios 

// randomly initialize balls
inline void init_random(std::vector<std::shared_ptr<Physical>>& objects)
{
    std::mt19937 rand_gen = RandGen::get();
    
    for (int i = 0; i < (rand_gen() % (MAX_BALLS - MIN_BALLS) + MIN_BALLS); i++)
        //for (int i = 0; i < 1; i++)
    {
        Ball ball;
        
        int r = 5 + rand_gen() % 7;
        ball.R = r;
        ball.p.x = (r + rand_gen()) % (WINDOW_X - r); // make sure balls dont spawn on the edges 
        ball.p.y = (r + rand_gen()) % (WINDOW_Y - r);
        ball.dir.x = (-5.f + (rand_gen() % 10)) / 3.f;
        ball.dir.y = (-5.f + (rand_gen() % 10)) / 3.f;
        ball.speed = (30 + rand_gen() % 30) * 1;
        
        objects.push_back(std::make_shared<Ball>(ball));
    }
}

inline void init_corner_bounce(std::vector<std::shared_ptr<Physical>>& objects)
{
    Ball ball;

    ball.R = 20;
    ball.p.x = 100;
    ball.p.y = 100;
    ball.dir.x = -1;
    ball.dir.y = -1;
    ball.speed = 500;

    objects.push_back(std::make_shared<Ball>(ball));

    ball.p.x = 300;
    ball.p.y = 300;
    ball.dir.x = 1;
    ball.dir.y = 1;

    objects.push_back(std::make_shared<Ball>(ball));
}

inline void init_chain(std::vector<std::shared_ptr<Physical>>& objects)
{
    float R = 20;

    Ball ball;

    ball.R = R;
    ball.p.x = 100;
    ball.p.y = 100;
    ball.dir.x = -1;
    ball.dir.y = 0;
    ball.speed = 500;
    objects.push_back(std::make_shared<Ball>(ball));

    ball.p.x += 2 * R;
    ball.dir.x = 0;
    ball.speed = 0;
    objects.push_back(std::make_shared<Ball>(ball));

    ball.p.x += 2 * R;
    objects.push_back(std::make_shared<Ball>(ball));

    ball.p.x += 2 * R;
    objects.push_back(std::make_shared<Ball>(ball));
}

inline void init_snooker(std::vector<std::shared_ptr<Physical>>& objects)
{
    float R = 20;

    Ball ball;
    std::mt19937 rand_gen = RandGen::get();

    // 0
    ball.R = R;
    ball.p.x = 1.f * WINDOW_X - R - 1;
    ball.p.y = 1.f * WINDOW_Y / 2;
    ball.dir.x = -1;
    ball.dir.y = 0;
    ball.speed = 500;
    objects.push_back(std::make_shared<Ball>(ball));

    //1 
    ball.p.x = 1.f * WINDOW_X / 2;
    ball.p.y = 1.f * WINDOW_Y / 2;
    ball.dir.x = 0;
    ball.speed = 0;
    objects.push_back(std::make_shared<Ball>(ball));

    for (int j = 2; j <= 5; ++j)
    {
        float sign = j % 2 == 0 ? 1.f : -1.f;

        ball.p.x -= R * 2 * std::cos(M_PI / 6);
        ball.p.y += R * 3 * sign;
        ball.R += 0.001f * (4 - rand_gen() % 5);
        
        for (int i = 0; i < j; ++i)
        {
            ball.p.y -= R * 2 * sign;
            objects.push_back(std::make_shared<Ball>(ball));
        }
    }
}

inline void init_angled1(std::vector<std::shared_ptr<Physical>>& objects)
{
    float R = 20;

    Ball ball;

    ball.R = R;
    ball.p.x = 1.f * WINDOW_X - R - 1;
    ball.p.y = 1.f * WINDOW_Y / 2;
    ball.dir.x = -1;
    ball.dir.y = 0;
    ball.speed = 250;
    objects.push_back(std::make_shared<Ball>(ball));

    ball.p.x = 1.f * WINDOW_X / 2;
    ball.p.y = 1.f * WINDOW_Y / 2;
    ball.p.y += R;
    ball.dir.x = 0;
    ball.speed = 0;
    objects.push_back(std::make_shared<Ball>(ball));
}

inline void init_angled2(std::vector<std::shared_ptr<Physical>>& objects)
{
    float R = 20;

    Ball ball;

    ball.R = R;
    ball.p.x = 1.f * WINDOW_X - R - 1;
    ball.p.y = 1.f * WINDOW_Y / 2;
    ball.dir.x = -1;
    ball.dir.y = 0;
    ball.speed = 250;
    objects.push_back(std::make_shared<Ball>(ball));

    ball.p.x = 1.f * WINDOW_X / 2;
    ball.p.y = 1.f * WINDOW_Y / 2;
    ball.p.y -= R * 1.5f;
    ball.dir.x = 0;
    ball.speed = 0;
    objects.push_back(std::make_shared<Ball>(ball));
}

inline void init_size(std::vector<std::shared_ptr<Physical>>& objects)
{
    float R = 20;

    Ball ball;

    ball.R = R;
    ball.p.x = 1.f * WINDOW_X - R - 1;
    ball.p.y = 1.f * WINDOW_Y / 2;
    ball.dir.x = -1;
    ball.dir.y = 0;
    ball.speed = 250;
    objects.push_back(std::make_shared<Ball>(ball));

    ball.R = R * 2;
    ball.p.x = 1.f * 0 + ball.R + 1;
    ball.p.y = 1.f * WINDOW_Y / 2;
    ball.dir.x = 1;
    ball.speed = 250;
    objects.push_back(std::make_shared<Ball>(ball));
}