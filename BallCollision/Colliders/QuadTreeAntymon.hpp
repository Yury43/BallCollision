#pragma once
#include <vector>

#include "collision_detector.hpp"
#include "antymon/Quadtree.h"

class QuadTreeAntymon : public CollisionDetector
{
public:
    QuadTreeAntymon(int l, int t, int r, int b, int max_elements, int max_depth);
    ~QuadTreeAntymon() override;

    void detect_collisions(std::vector<int>& proxy, float x, float y, float r, int id) override;
    void add(float x, float y, float r, int i) override;

private:
    Quadtree* qt = nullptr;
};
