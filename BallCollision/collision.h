#pragma once

#include "physicals.h"

// Collision class considers and handles system of colliding agents as a whole

class Collision
{
public:

    const uint64_t cid;

    Collision(const std::shared_ptr<Physical>& p1, const std::shared_ptr<Physical>& p2);

    bool are_touching() const;
    void mark_started() const;
    void mark_finished() const;

    void handle() const;

private:
    std::shared_ptr<Physical> party1;
    std::shared_ptr<Physical> party2;

    static uint64_t make_id(const std::shared_ptr<Physical>& p1, const std::shared_ptr<Physical>& p2);
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

