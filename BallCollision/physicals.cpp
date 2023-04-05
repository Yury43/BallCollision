#include "physicals.h"
#include <algorithm>
#include <iostream>
#include <assert.h>
#include "math.h"

uint32_t Physical::next_id = 0;



//static bool are_touching(const Line& line, const Ball& ball)
//{
//    return norm(project(line, ball.p) - ball.p) <= ball.R;
//}


static bool are_touching(const Ball& b1, const Ball& b2)
{
    return norm(b1.p - b2.p) <= (b1.R + b2.R);
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
        return are_touching(*other_as_ball, *this);
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

    auto p0 = impulse();

    sf::Vector2f v2 = sf::Vector2f(0, 0);
    sf::Vector2f r2 = sf::Vector2f(0, 0);

    if (reactions.size() > 1)
    {
        std::cout << "reactions: " << reactions.size() << std::endl;
    }

    assert(reactions.size() == 1);

    v2 = reactions.front().velocity_delta;
    r2 = reactions.front().position_delta;

    //std::for_each(reactions.begin(), reactions.end(), [&dp](const Reaction& item) {dp += item.impulse_delta; });
    //dp /= static_cast<float>(reactions.size()); // this doesn make sence, the resulting impulse cound be averaged, but delta should be summed 
    //std::for_each(reactions.begin(), reactions.end(), [&dr](const Reaction& item) {dr += item.position_delta; });
    //dr /= static_cast<float>(reactions.size()); // this doesn make sence, the resulting position cound be averaged, but delta should be summed 

    //std::cout << "dp: " << dp << " ; dr: " << dr << std::endl;


    reactions.clear();

    p = r2;
    speed = norm(v2);
    dir = normalized(v2);
}

void Ball::handle_collision(Physical* other)
{
    //auto other_line = dynamic_cast<Line*>(other);
    //if (other_line != nullptr)
    //{
    //    //std::cout << "wall" << std::endl;
    //    // calculate impulse delta 
    //    auto contact_point = project(*other_line, p);
    //    auto ball_to_wall = contact_point - p;

    //    sf::Vector2f p0 = impulse();
    //    sf::Vector2f pn = project(p0, ball_to_wall);

    //    auto dp = -2.f * pn;

    //    // correct position, placing ball on the point of contact 
    //    auto dr = -normalized(ball_to_wall) * (R - norm(ball_to_wall));

    //    if (norm(dp) > 1e-6)
    //    {
    //        //std::cout << "dp: " << dp << " ; dr: " << dr << std::endl;
    //        reactions.push_back({ dp, dr });
    //    }

    //    return;
    //}

    auto other_ball = dynamic_cast<Ball*>(other);
    if (other_ball != nullptr)
    {
        float m1 = mass();
        float m2 = other_ball->mass();

        auto r1 = p;
        auto r2 = other_ball->p;

        auto v1 = velocity();
        auto v2 = other_ball->velocity();

        // correct positions 

        auto c = (r1 + r2) / 2.f;
        auto drc1 = r1 - c;
        auto drc2 = r2 - c;
        auto dr1 = normalized(drc1) * (R - norm(drc1) + 1e-2f);
        auto dr2 = normalized(drc2) * (R - norm(drc2) + 1e-2f);

        r1 = r1 + dr1;
        r2 = r2 + dr2;

        // calculate velocity deltas 

        auto t = normalized(r2 - r1);
        auto n = sf::Vector2f(t.y, t.x);
        
        float v1t = dot(v1, t);
        float v2t = dot(v2, t);

        float v1n = norm(v1 - v1t * t);
        float v2n = norm(v2 - v2t * t);

        float v1t2 = (m2 * v2t * 2.f + v1t * (m1 - m2)) / (m1 + m2);
        float v2t2 = (m1 * v1t * 2.f + v2t * (m2 - m1)) / (m1 + m2);

        auto v12 = t * v1t2 + n * v1n;
        auto v22 = t * v2t2 + n * v2n;

        if (norm(v12) > 1e-6 || norm(dr1) > 1e-6)
        {
            reactions.push_back({ v12, r1 });
        }

        if (norm(v22) > 1e-6 || norm(dr2) > 1e-6)
        {
            other_ball->reactions.push_back({ v22, r2 });
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