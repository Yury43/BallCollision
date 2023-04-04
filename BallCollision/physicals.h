#pragma once
#include <cstdint>
#include <SFML/System/Vector2.hpp>
#include <vector>
#include "math.h"

struct Physical
{
    const uint32_t id;
    Physical() : id(next_id++) {};

    virtual ~Physical()
    {

    }

    virtual bool is_touching(const Physical* other) const = 0;

private:
    static uint32_t next_id;
};


struct Ball : public Physical
{
    sf::Vector2f p = { 0, 0 };
    sf::Vector2f dir = { 0, 0 };
    float r = 0;
    float speed = 0;
    sf::Color color = sf::Color::White;

    Ball() : Physical() {};

    virtual ~Ball()
    {

    }

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

    std::vector<sf::Vector2f> reactions = {};

    bool is_touching(const Physical* other) const override;
};


struct Line : public Physical
{
    sf::Vector2f p1;
    sf::Vector2f p2;
    sf::Vector2f n;

    Line(const sf::Vector2f& _p1, const sf::Vector2f& _p2) : Physical(), p1(_p1), p2(_p2), n(normalized(p2 - p1))
    {

    }

    virtual ~Line()
    {

    }

    bool is_touching(const Physical* other) const override;
};

static sf::Vector2f project(const Line& l, const sf::Vector2f& p)
{
    const auto& a = l.p1;
    const auto& n = l.n;
    auto d = p - a;
    auto pr = d - (dot(d, n) * n);
    return pr + a;
}

static sf::Vector2f project(const sf::Vector2f& a, const sf::Vector2f& b)
{
    return dot(a, b) / norm(b) * normalized(b);
}

static float dist(const Line& l, const sf::Vector2f& p)
{
    return norm(project(l, p));
}


static bool are_touching(const Line& line, const Ball& ball)
{
    return norm(project(line, ball.p) - ball.p) <= ball.r;
}


static bool are_touching(const Ball& b1, const Ball& b2)
{
    return norm(b1.p - b2.p) <= (b1.r + b2.r);
}


