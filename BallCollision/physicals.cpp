#include "physicals.h"
#include <algorithm>
#include <iostream>
#include <assert.h>
#include "vector_math.h"

uint32_t Physical::next_id = 0;



//static bool are_touching(const Line& line, const Ball& ball)
//{
//    return norm(project(line, ball.p) - ball.p) <= ball.R;
//}

constexpr float DELTA = 1e-3f;

static bool are_touching(const Ball& b1, const Ball& b2)
{
    return dist(b1.p, b2.p) < b1.R + b2.R + DELTA;
}

bool Ball::is_touching(const Physical* other) const
{
    //const Line* other_as_line = dynamic_cast<const Line*>(other);
    //if (other_as_line != nullptr)
    //{
    //    return are_touching(*other_as_line, *this);
    //}
    const Ball* other_as_ball = dynamic_cast<const Ball*>(other);
    if (other_as_ball != nullptr)
    {
        return are_touching(*this, *other_as_ball);
    }
    throw std::logic_error("Not implemented");
}

void Ball::mark_colliding(bool is_colliding)
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

    assert(reactions.size() == 1);

    Reaction reaction = reactions.front();

    //std::for_each(reactions.begin(), reactions.end(), [&dp](const Reaction& item) {dp += item.impulse_delta; });
    //dp /= static_cast<float>(reactions.size()); // this doesn make sence, the resulting impulse cound be averaged, but delta should be summed 
    //std::for_each(reactions.begin(), reactions.end(), [&dr](const Reaction& item) {dr += item.position_delta; });
    //dr /= static_cast<float>(reactions.size()); // this doesn make sence, the resulting position cound be averaged, but delta should be summed 

    //std::cout << "dp: " << dp << " ; dr: " << dr << std::endl;

    reactions.clear();

    p = reaction.corrected_position;
    speed = norm(reaction.velocity);
    dir = normalized(reaction.velocity);
}

void Ball::handle_collision(Physical* other)
{
    auto other_ball = dynamic_cast<Ball*>(other);
    if (other_ball != nullptr)
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

        auto dv1 = v1 - (m1 * 2 / (m1 + m2)) * dot(v1 - v2, r12) / std::powf(norm(r12), 2) * r12;
        auto dv2 = v2 - (m2 * 2 / (m1 + m2)) * dot(v2 - v1, r21) / std::powf(norm(r21), 2) * r21;

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