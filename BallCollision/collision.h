#pragma once

#include <memory>

#include "physicals.h"

// Collision class considers and handles system of colliding agents as a whole

class Collision
{
public:

    const uint64_t cid;

    Collision(Physical* p1, Physical* p2);

    bool are_touching() const;
    void mark_started() const;
    void mark_finished() const;

    void handle() const;

private:
    Physical* party1;
    Physical* party2;

    static uint64_t make_id(const Physical* p1, const Physical* p2);
};

// support for std::unordered_* containers 

namespace std
{
    template <>
    struct hash<Collision>
    {
        std::size_t operator()(const Collision& c) const noexcept
        {
            return c.cid;
        }
    };
}

static inline bool operator == (const Collision& left, const Collision& right)
{
    return left.cid == right.cid;
}

