#pragma once
#include <memory>
#include <optional>

#include "physicals.h"


class CollisionDetector
{
public:
    
    CollisionDetector() = default;
    virtual ~CollisionDetector() = default;
    
    CollisionDetector(const CollisionDetector&) = delete;
    CollisionDetector(CollisionDetector &&) = delete;
    CollisionDetector& operator =(CollisionDetector &&) = delete;
    // CollisionDetector operator =(CollisionDetector) = delete;
    auto operator=(const CollisionDetector &) -> CollisionDetector & = delete;
    
    virtual void detect_collisions(
        const std::vector<std::shared_ptr<Collidable>> & agents,
        const int index,
        std::vector<std::pair<uint32_t, uint32_t>>& colliding_pairs,
        int* collision_test_counter = nullptr
    ) = 0;

    virtual void add(const Collidable* c) = 0;
};





