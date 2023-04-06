#pragma once

#include "physicals.h"

class Collision
{
public:

    const uint64_t cid;

    Collision(const std::shared_ptr<Physical>& p1, const std::shared_ptr<Physical>& p2);

    bool are_touching() const;
    void mark_started() const;
    void mark_finished() const;

    void handle();

private:
    std::shared_ptr<Physical> party1;
    std::shared_ptr<Physical> party2;

    static uint64_t make_id(const std::shared_ptr<Physical>& p1, const std::shared_ptr<Physical>& p2);
};

namespace std
{
    template <>
    struct hash<Collision>
    {
        std::size_t operator()(const Collision& c) const
        {
            return c.cid;
        }
    };
}

inline bool operator == (const Collision& left, const Collision& right)
{
    return left.cid == right.cid;
}

