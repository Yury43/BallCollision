#include "Physicals.h"
#include <algorithm>
#include <iostream>
#include "math.h"

uint32_t Physical::next_id = 0;



static bool are_touching(const Line& line, const Ball& ball)
{
    return norm(project(line, ball.p) - ball.p) <= ball.r;
}


static bool are_touching(const Ball& b1, const Ball& b2)
{
    return norm(b1.p - b2.p) <= (b1.r + b2.r);
}

bool Ball::is_touching(const Physical* other) const
{
    const Line* other_as_line = dynamic_cast<const Line*>(other);
    if (other_as_line != nullptr)
    {
        return are_touching(*other_as_line, *this);
    }
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

    sf::Vector2f dp = sf::Vector2f(0, 0);
    std::for_each(reactions.begin(), reactions.end(), [&dp](const Reaction& item) {dp += item.impulse_delta; });
    //dp /= static_cast<float>(reactions.size()); // this doesn make sence, the resulting impulse cound be averaged, but delta should be summed 

    sf::Vector2f dr = sf::Vector2f(0, 0);
    std::for_each(reactions.begin(), reactions.end(), [&dr](const Reaction& item) {dr += item.position_delta; });
    //dr /= static_cast<float>(reactions.size()); // this doesn make sence, the resulting position cound be averaged, but delta should be summed 

    //std::cout << "dp: " << dp << " ; dr: " << dr << std::endl;

    reactions.clear();

    auto p2 = p0 + dp;

    auto adp = std::abs(norm(p0) - norm(p2));

    auto v = p2 / mass();
    speed = norm(v);
    dir = normalized(v);
    p = p + dr;
}

void Ball::handle_collision(Physical* other)
{
    auto other_line = dynamic_cast<Line*>(other);
    if (other_line != nullptr)
    {
        //std::cout << "wall" << std::endl;
        // calculate impulse delta 
        auto contact_point = project(*other_line, p);
        auto ball_to_wall = contact_point - p;

        sf::Vector2f p0 = impulse();
        sf::Vector2f pn = project(p0, ball_to_wall);

        auto dp = -2.f * pn;

        // correct position, placing ball on the point of contact 
        auto dr = -normalized(ball_to_wall) * (r - norm(ball_to_wall));

        if (norm(dp) > 1e-6)
        {
            //std::cout << "dp: " << dp << " ; dr: " << dr << std::endl;
            reactions.push_back({ dp, dr });
        }

        return;
    }

    auto other_ball = dynamic_cast<Ball*>(other);
    if (other_ball != nullptr)
    {
        //std::cout << "ball" << std::endl;
        // calculate impulse deltas 
        auto v1 = velocity();
        auto v2 = other_ball->velocity();

        auto m1 = mass();
        auto m2 = other_ball->mass();

        auto dv1 = (m2 * v2 * 2.f + v1 * (m1 - m2)) / (m1 + m2) - v1;
        auto dv2 = (m1 * v1 * 2.f + v2 * (m2 - m1)) / (m1 + m2) - v2;

        // correct positions 
        const auto& r1 = p;
        const auto& r2 = other_ball->p;
        auto c = (r1 + r2) / 2.f;
        auto drc1 = r1 - c;
        auto drc2 = r2 - c;
        auto dr1 = normalized(drc1) * (r - norm(drc1));
        auto dr2 = normalized(drc2) * (r - norm(drc2));

        if (norm(dv1) > 1e-6)
        {
            reactions.push_back({ dv1 * m1, dr1 });
        }

        if (norm(dv2) > 1e-6)
        {
            other_ball->reactions.push_back({ dv2 * m2, dr2 });
        }

        return;
    }

    throw std::logic_error("Not implemented");
}


bool Line::is_touching(const Physical* other) const
{
    const Ball* other_as_ball = dynamic_cast<const Ball*>(other);
    if (other_as_ball != nullptr)
    {
        return are_touching(*this, *other_as_ball);
    }
    throw std::logic_error("Not implemented");
}


void Line::handle_collision(Physical* other)
{
    auto other_line = dynamic_cast<Line*>(other);
    if (other_line != nullptr)
    {
        return;
    }

    auto other_ball = dynamic_cast<Ball*>(other);
    if (other_ball != nullptr)
    {
        other_ball->handle_collision(this);
        return;
    }

    throw std::logic_error("Not implemented");
}