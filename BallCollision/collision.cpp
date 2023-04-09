#include "collision.h"


Collision::Collision(
    const std::shared_ptr<Physical>& p1,
    const std::shared_ptr<Physical>& p2)
    : cid(make_id(p1, p2)), party1(p1), party2(p2)
{

}

bool Collision::are_touching() const
{
    return party1->is_touching(party2.get());
}

void Collision::handle() const
{
    return party1->handle_collision(party2.get());
}

void Collision::mark_started() const
{
    party1->mark_colliding(true);
    party2->mark_colliding(true);
}

void Collision::mark_finished() const
{
    party1->mark_colliding(false);
    party2->mark_colliding(false);
}

uint64_t Collision::make_id(const std::shared_ptr<Physical>& p1, const std::shared_ptr<Physical>& p2)
{
    int high = p1->id;
    int low = p2->id;

    if (high > low) // make collisions of same objects identical 
    {
        std::swap(low, high);
    }

    return (((uint64_t)high) << 32) | ((uint64_t)low);
}
