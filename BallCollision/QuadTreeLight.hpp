#pragma once
#include "collision_detector.h"
#include "constants.h"
#include "QuadTreeQuadrant.h"


// quadrant is not initialized intentionally, to create node before knowing it 
class LightQuadTreeNode
{
public:
    /* int speedup is not too great but creates a risk of content collisions when going too deep,
     * requiring the ability to store multiple content elements inside a single node */
    
    // typedef Quadrant<int> Quad;
    typedef Quadrant<float> Quad; 
    // typedef CenteredQuadrant<float> Quad; // appears to be slower 
    
    LightQuadTreeNode() = default;
    explicit LightQuadTreeNode(const Quad & q_) : quadrant(q_) {}
    explicit LightQuadTreeNode(Quad & q_) : quadrant(q_) {}
    
    LightQuadTreeNode(const LightQuadTreeNode& _) = default;
    LightQuadTreeNode(LightQuadTreeNode &&_) = default;
    LightQuadTreeNode& operator =(LightQuadTreeNode && _) = default;
    
    LightQuadTreeNode operator =(const LightQuadTreeNode & _) = delete;
    
private:

    Quad quadrant;
    
    int first_leaf = -1; // we dont need to store all the leafs, they are 

    const Collidable* content = nullptr;

    friend class LightQuadTree;
};


class LightQuadTree : public CollisionDetector
{
public:

    using Quad = LightQuadTreeNode::Quad;

    LightQuadTree(float l, float t, float r, float b);
    
    LightQuadTree(const LightQuadTree& _) = delete;
    LightQuadTree(LightQuadTree &&_) = delete;
    LightQuadTree& operator =(LightQuadTree && _) = delete;
    LightQuadTree operator =(const LightQuadTree & _) = delete;

    ~LightQuadTree() override = default;
    
    void detect_collisions(
        const std::vector<std::shared_ptr<Collidable>> & agents,
        const int index,
        std::vector<std::pair<uint32_t, uint32_t>>& colliding_pairs,
        int* collision_test_counter = nullptr
    ) override;
    
private:

    std::array<LightQuadTreeNode, static_cast<size_t>(MAX_BALLS * 4 * 2)> nodes;
    int next_placed_node = 0;
    float max_span = 0;
        
    int create_new_node(const Quad& q);
    int get_next_node(int pos, sf::Vector2f loc);
    void push(int pos, const Collidable* item);
    void query_range(int pos, const sf::Vector2f & loc, float R, std::vector<const Collidable*>& collection) const;
    void query_range(int pos, const Quad & loc, std::vector<const Collidable*>& collection) const;
};
