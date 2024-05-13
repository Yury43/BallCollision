#pragma once
#include "collision_detector.h"
#include "antymon/Quadtree.h"

class QuadTreeAntymon : public CollisionDetector
{
public:
    QuadTreeAntymon(int l, int t, int r, int b, int max_elements, int max_depth);
    ~QuadTreeAntymon() override;

    void detect_collisions(
        const std::vector<std::shared_ptr<Physical>> & agents,
        const int index,
        std::vector<std::pair<uint32_t, uint32_t>>& colliding_pairs,
        int* collision_test_counter = nullptr
    ) override;
    
    void reset_agents(const std::vector<std::shared_ptr<Physical>> & agents);

private:
    Quadtree* qt = nullptr;
};
