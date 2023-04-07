#pragma once

#include <SFML/System/Vector2.hpp>
#include <vector>
#include "vector_math.h"

// Abstract physical object parenting Ball and (previously) Line 

class Physical
{
public:
    const uint32_t id;
    Physical() : id(next_id++) 
    { 
        //std::cout << "created Physical #" << id << std::endl; 
    }

    Physical(const Physical &_) = delete;
    Physical(Physical &&_) = delete;
    
    virtual ~Physical() = default;

    virtual bool is_touching(const Physical* other) const = 0;
    virtual void mark_colliding(bool is_colliding) {}
    virtual void handle_collision(Physical* other) = 0;

private:
    static uint32_t next_id;
};

// The result of collision applied on object 
struct Reaction
{
    sf::Vector2f velocity = { 0, 0 };
    sf::Vector2f corrected_position = { 0, 0 };
};


class Ball : public Physical
{
public:
    sf::Vector2f p = { 0, 0 };
    sf::Vector2f dir = { 0, 0 };
    float R = 0;
    float speed = 0;
    sf::Color color = sf::Color::White;

    Ball() = default;

    Ball(const Ball& other);
    Ball(Ball &&_) = delete;
    Ball operator =(const Ball & _) = delete;
    Ball operator =(const Ball && _) = delete;
    
    ~Ball() override = default;

    bool is_touching(const Physical* other) const override;

    void mark_colliding(bool is_colliding) override;
    void handle_collision(Physical* other) override;

    float mass() const;
    sf::Vector2f velocity() const;
    sf::Vector2f momentum() const;
    double energy() const;

    bool test_and_handle_wall_collision(float left, float top, float right, float bottom);
    void apply_reactions();

private:
    std::vector<Reaction> reactions = {};
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

//static sf::Vector2f project(const sf::Vector2f& a, const sf::Vector2f& b)
//{
//    return dot(a, b) / norm(b) * normalized(b);
//}

//static float dist(const Line& l, const sf::Vector2f& p)
//{
//    return norm(project(l, p));
//}





