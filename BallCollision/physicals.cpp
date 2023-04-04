#include "Physicals.h"


uint32_t Physical::next_id = 0;


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


bool Line::is_touching(const Physical* other) const
{
    const Ball* other_as_ball = dynamic_cast<const Ball*>(other);
    if (other_as_ball != nullptr)
    {
        return are_touching(*this, *other_as_ball);
    }
    throw std::logic_error("Not implemented");
}

