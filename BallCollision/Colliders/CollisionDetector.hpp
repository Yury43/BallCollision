#pragma once
#include <vector>

struct ContentItem
{
    float x;
    float y;
    float r;
    int i;
};

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
    
    virtual void detect_collisions(std::vector<int>& proxy, float x, float y, float r, int id) = 0;
    virtual void add(float x, float y, float r, int i) = 0;
};





