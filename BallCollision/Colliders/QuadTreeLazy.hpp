#pragma once
#include <list>
#include <memory>
#include <vector>

#include "collision_detector.hpp"
#include "QuadTreeQuadrant.hpp"


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
    LazyQuadTreeNode* get_next_node(float x, float y);

    void add_content(const ContentItem* item);
    bool got_content() const;
    size_t content_size() const;
    void collect_content(std::vector<int>& collection) const;
    void push_current_content();
    
    LazyQuadTreeNode* push(const ContentItem* item);
    size_t count_nodes() const ;
    size_t count_items() const ;
    void query_range(std::vector<int>& collection, float x, float y, float r) const;
    void query_range(std::vector<int>& collection, const Quad & loc) const;

private:

    Quad quadrant;
    Quad::Subdivision subquadrants;
    std::array<std::unique_ptr<LazyQuadTreeNode>, 4> leaves;
    // std::array<LazyQuadTreeNode*, 4> leaves = {0, 0, 0, 0};
    bool got_leaves = false;
    std::vector<const ContentItem*> content;
    // const Physical* content = nullptr;
};


class LazyQuadTree : public CollisionDetector
{
public:
    LazyQuadTree(int l, int t, int r, int b);
    
    LazyQuadTree(const LazyQuadTree& _) = delete;
    LazyQuadTree(LazyQuadTree &&_) = delete;
    LazyQuadTree& operator =(LazyQuadTree && _) = delete;
    LazyQuadTree operator =(const LazyQuadTree & _) = delete;

    ~LazyQuadTree() override = default;

    void detect_collisions(std::vector<int>& proxy, float x, float y, float r, int id) override;
    void add(float x, float y, float r, int i) override;

private:
    
    LazyQuadTreeNode root;
    float max_span = 0;
    std::list<ContentItem> content;
};