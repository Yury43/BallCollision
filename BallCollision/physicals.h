#pragma once
#include <cstdint>
#include <SFML/System/Vector2.hpp>
#include <vector>
#include "math.h"
#include <iostream>

class Physical
{
public:
    const uint32_t id;
    Physical() : id(next_id++) 
    { 
        //std::cout << "created Physical #" << id << std::endl; 
    };


    virtual ~Physical(){}

    virtual bool is_touching(const Physical* other) const = 0;
    virtual void mark_colliding(bool is_colliding) {};
    virtual void handle_collision(Physical* other) = 0;

private:
    static uint32_t next_id;
};

struct Reaction
{
    sf::Vector2f velocity_delta = { 0, 0 };
    sf::Vector2f position_delta = { 0, 0 };
};

class Ball : public Physical
{
public:
    sf::Vector2f p = { 0, 0 };
    sf::Vector2f dir = { 0, 0 };
    float R = 0;
    float speed = 0;
    sf::Color color = sf::Color::White;
    std::vector<Reaction> reactions = {};

    Ball() : Physical() {};

    Ball(const Ball& other) : Physical()
    {
        this->p = other.p;
        this->dir = other.dir;
        this->R = other.R;
        this->speed = other.speed;
        this->color = other.color;
        this->reactions = other.reactions;
    }

    virtual ~Ball()
    {

    }

    float mass() const
    {
        return R * R * R; // consider mass to be a function of volume just to make interactions a little bit easier to perceive and comprehend
    }

    sf::Vector2f velocity() const
    {
        return speed * dir;
    }

    sf::Vector2f impulse() const
    {
        return velocity() * mass();
    }

    double Energy()
    {
        return std::pow(1. * norm(velocity()), 2) * mass() / 2;
    }

    bool is_touching(const Physical* other) const override;
    void mark_colliding(bool is_colliding) override;
    void handle_collision(Physical* other) override;
    void apply_reactions();

    bool test_wall_collision(int left, int top, int right, int bottom)
    {
        bool collided = false;

        if (p.x - R < left)
        {
            dir.x *= -1;
            p.x = left + R;
            collided = true;
        }
        else if (p.x + R > right)
        {
            dir.x *= -1;
            p.x = right - R;
            collided = true;
        }

        if (p.y - R < top)
        {
            dir.y *= -1;
            p.y = top + R;
            collided = true;
        }
        else if (p.y + R > bottom)
        {
            dir.y *= -1;
            p.y = bottom - R;
            collided = true;
        }

        return collided;
    }
};


//class Line: public Physical
//{
//public:
//    sf::Vector2f p1;
//    sf::Vector2f p2;
//    sf::Vector2f n;
//
//    Line(const sf::Vector2f& _p1, const sf::Vector2f& _p2) 
//        : Physical(), p1(_p1), p2(_p2), n(normalized(p2 - p1))
//    {
//
//    }
//
//    virtual ~Line(){}
//
//    bool is_touching(const Physical* other) const override;
//    void handle_collision(Physical* other) override;
//};

//static sf::Vector2f project(const Line& l, const sf::Vector2f& p)
//{
//    const auto& a = l.p1;
//    const auto& n = l.n;
//    auto d = p - a;
//    auto pr = d - (dot(d, n) * n);
//    return pr + a;
//}

static sf::Vector2f project(const sf::Vector2f& a, const sf::Vector2f& b)
{
    return dot(a, b) / norm(b) * normalized(b);
}

//static float dist(const Line& l, const sf::Vector2f& p)
//{
//    return norm(project(l, p));
//}





