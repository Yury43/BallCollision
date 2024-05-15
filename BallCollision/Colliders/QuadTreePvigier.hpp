#pragma once
#include <functional>

#include "collision_detector.hpp"
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
    
    explicit QuadTreePvigier();
    
    void detect_collisions(
        std::vector<int>& proxy, const float x, const float y, const float r, const int id
    ) override;

private:
    std::function<quadtree::Box<float>(Node*)> getBox;

    std::vector<Node> nodes;
    std::unique_ptr<quadtree::Quadtree<Node*, decltype(getBox)>> t;
    quadtree::Box<float> box;
    
};
