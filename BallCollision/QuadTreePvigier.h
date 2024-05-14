#pragma once
#include <functional>

#include "collision_detector.h"
#include "pvigier/Box.h"
#include "pvigier/Quadtree.h"

struct Node
{
    quadtree::Box<float> box;
    uint32_t id;
};


class QuadTreePvigier : CollisionDetector
{
public:
    
    explicit QuadTreePvigier(const std::vector<std::shared_ptr<Collidable>> & objects);
    
    void detect_collisions(
        const std::vector<std::shared_ptr<Collidable>> & agents,
        const int index,
        std::vector<std::pair<uint32_t, uint32_t>>& colliding_pairs,
        int* collision_test_counter = nullptr
    ) override;

private:
    std::function<quadtree::Box<float>(Node*)> getBox;

    std::vector<Node> nodes;
    std::unique_ptr<quadtree::Quadtree<Node*, decltype(getBox)>> t;
    quadtree::Box<float> box;
    
};
