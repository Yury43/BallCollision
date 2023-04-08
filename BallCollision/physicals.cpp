
#include <algorithm>
#include <iostream>
#include <cassert>
#include "physicals.h"

#include "ball.h"
#include "vector_math.h"

uint32_t Physical::next_id = 0;


//static bool are_touching(const Line& line, const Ball& ball)
//{
//    return norm(project(line, ball.p) - ball.p) <= ball.R;
//}

constexpr float DELTA = 1e-3f;

// static bool are_touching(const Ball& b1, const Ball& b2)
// {
//     return b1.is_touching(b2) dist(b1.p, b2.p) < b1.R + b2.R + DELTA;
// }

bool Ball::is_touching(const Physical* other) const
{
    if (other->id == this->id)
        return false;
    
    //const Line* other_as_line = dynamic_cast<const Line*>(other);
    //if (other_as_line != nullptr)
    //{
    //    return are_touching(*other_as_line, *this);
    //}
    auto other_ball = dynamic_cast<const Ball*>(other);
    if (other_ball)
    {
        return dist(p, other_ball->p) < R + other_ball->R + DELTA;
    }
    throw std::logic_error("Not implemented");
}


void Ball::mark_colliding(const bool is_colliding)
{
    color = is_colliding ? sf::Color::Red : sf::Color::White;
}

void Ball::apply_reactions()
{
    if (reactions.empty())
    {
        return;
    }

    if (reactions.size() > 1)
    {
        std::cout << "reactions: " << reactions.size() << std::endl;
    }

    sf::Vector2f v2(0, 0);
    sf::Vector2f r2(0, 0);
    float n_reactions = static_cast<float>(reactions.size());
    
    std::for_each(reactions.begin(), reactions.end(), [&v2](const Reaction& item) {v2 += item.velocity; });
    v2 /= n_reactions;
    
    std::for_each(reactions.begin(), reactions.end(), [&r2](const Reaction& item) {r2 += item.corrected_position; });
    r2 /= n_reactions;  

    reactions.clear();

    p = r2;
    speed = norm(v2);
    dir = normalized(v2);
}

void Ball::handle_collision(Physical* other)
{
    auto other_ball = dynamic_cast<Ball*>(other);
    if (other_ball)
    {
        auto r1 = p;
        auto r2 = other_ball->p;

        // correct positions 
        
        auto dr1 = sf::Vector2f(0, 0);
        auto dr2 = sf::Vector2f(0, 0);

        if (norm(r1 - r2) < R + other_ball->R + DELTA)
        {
            auto c = (r1 + r2) / 2.f;
            auto drc1 = r1 - c;
            auto drc2 = r2 - c;
            dr1 = normalized(drc1) * (R - norm(drc1) + DELTA * 2);
            dr2 = normalized(drc2) * (other_ball->R - norm(drc2) + DELTA * 2);

            r1 = r1 + dr1;
            r2 = r2 + dr2;
        }

        // calculate velocity deltas 

        float m1 = mass();
        float m2 = other_ball->mass();

        auto v1 = velocity();
        auto v2 = other_ball->velocity();

        auto r12 = r1 - r2;
        auto r21 = -r12;

        auto dv1 = v1 - (m2 * 2 / (m1 + m2)) * dot(v1 - v2, r12) / std::powf(norm(r12), 2) * r12;
        auto dv2 = v2 - (m1 * 2 / (m1 + m2)) * dot(v2 - v1, r21) / std::powf(norm(r21), 2) * r21;

        if (norm(dv1) > 1e-5f || norm(dr1) > DELTA)
        {
            reactions.push_back({ dv1, r1 });
        }

        if (norm(dv2) > 1e-5f || norm(dr2) > DELTA)
        {
            other_ball->reactions.push_back({ dv2, r2 });
        }

        return;
    }

    throw std::logic_error("Not implemented");
}

void Ball::update_position(const float deltaTime)
{
    auto dr = velocity() * deltaTime;
    p.x += dr.x;
    p.y += dr.y;
}

std::shared_ptr<sf::Shape> Ball::get_drawing_shape() 
{
    sf::CircleShape shape;
    shape.setRadius(R);
    shape.setPosition(p.x - R, p.y - R); // consider ball.p to be the center
    shape.setFillColor(color);
    return std::make_shared<sf::CircleShape>(std::move(shape));
}

Ball::Ball(const Ball& other)
{
    this->p = other.p;
    this->dir = other.dir;
    this->R = other.R;
    this->speed = other.speed;
    this->color = other.color;
    this->reactions = other.reactions;
}

float Ball::mass() const 
{
    return R * R * R; // consider mass to be a function of volume just to make interactions a little bit easier to perceive and comprehend
}

sf::Vector2f Ball::velocity() const 
{
    return speed * dir;
}

sf::Vector2f Ball::momentum() const 
{
    return velocity() * mass();
}

double Ball::energy() const
{
    return std::pow(norm(velocity()), 2) * static_cast<double>(mass()) / 2;
}

bool Ball::test_and_handle_wall_collision(const float left, const float top, const float right, const float bottom)
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

//bool Line::is_touching(const Physical* other) const
//{
//    const Ball* other_as_ball = dynamic_cast<const Ball*>(other);
//    if (other_as_ball != nullptr)
//    {
//        return are_touching(*this, *other_as_ball);
//    }
//    throw std::logic_error("Not implemented");
//}
//
//
//void Line::handle_collision(Physical* other)
//{
//    auto other_line = dynamic_cast<Line*>(other);
//    if (other_line != nullptr)
//    {
//        return;
//    }
//
//    auto other_ball = dynamic_cast<Ball*>(other);
//    if (other_ball != nullptr)
//    {
//        other_ball->handle_collision(this);
//        return;
//    }
//
//    throw std::logic_error("Not implemented");
//}