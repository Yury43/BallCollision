#pragma once
#include "collision_detector.h"
#include "physicals.h"
#include "QuadTreeQuadrant.h"

class LazyQuadTreeNode
{
public:

    ~LazyQuadTreeNode()
    {
        // for (int i = 0; i < leaves.size(); ++i)
        // {
        //     if (leaves[i])
        //     {
        //         delete leaves[i];
        //     }
        // }
    }
    // typedef Quadrant<float> Quad;
    typedef Quadrant<int> Quad;
    
    explicit LazyQuadTreeNode(const Quad & q_);
    LazyQuadTreeNode* get_next_node(sf::Vector2f loc);

    void add_content(const Collidable* item);
    bool got_content() const;
    size_t content_size() const;
    void collect_content(std::vector<const Collidable*>& collection) const;
    void push_current_content();
    
    LazyQuadTreeNode* push(const Collidable* item);
    size_t count_nodes() const ;
    size_t count_items() const ;
    void query_range(const sf::Vector2f & loc, float R, std::vector<const Collidable*>& collection) const;
    void query_range(const Quad & loc, std::vector<const Collidable*>& collection) const;

private:

    Quad quadrant;
    Quad::Subdivision subquadrants;
    std::array<std::unique_ptr<LazyQuadTreeNode>, 4> leaves;
    // std::array<LazyQuadTreeNode*, 4> leaves = {0, 0, 0, 0};
    bool got_leaves = false;
    std::vector<const Collidable*> content;
    // const Collidable* content = nullptr;
};

class LightQuadTreeNode;


class LazyQuadTree : public CollisionDetector
{
public:
    LazyQuadTree(int l, int t, int r, int b);
    LazyQuadTree(const LazyQuadTree& _) = delete;
    LazyQuadTree(LazyQuadTree &&_) = delete;
    LazyQuadTree& operator =(LazyQuadTree && _) = delete;
    LazyQuadTree operator =(const LazyQuadTree & _) = delete;

    ~LazyQuadTree() override = default;

    void detect_collisions(
        const std::vector<std::shared_ptr<Collidable>> & agents,
        const int index,
        std::vector<std::pair<uint32_t, uint32_t>>& colliding_pairs,
        int* collision_test_counter = nullptr
    ) override;
    
    void add(const Collidable* c) override;

private:
    
    LazyQuadTreeNode root;
    float max_span = 0;
};