#pragma once
#include <list>

#include "collision_detector.hpp"
#include "QuadTreeQuadrant.hpp"


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

    const ContentItem* content = nullptr;

    template<size_t N_ELEMENTS>
    friend class LightQuadTree;
};

template<size_t N>
class LightQuadTree : public CollisionDetector
{
public:

    using Quad = LightQuadTreeNode::Quad;

    LightQuadTree(float l, float t, float r, float b)
    {
        // nodes.reserve(objects.size());
        create_new_node(Quad{l, t, r, b});
    }
    
    LightQuadTree(const LightQuadTree& _) = delete;
    LightQuadTree(LightQuadTree &&_) = delete;
    LightQuadTree& operator =(LightQuadTree && _) = delete;
    LightQuadTree operator =(const LightQuadTree & _) = delete;

    ~LightQuadTree() override = default;
    
    void detect_collisions(std::vector<int>& proxy, const float x, const float y, const float r, const int id)
    {
        query_range(proxy, 0, x, y, r + max_span + 1);
    }

    void add(float x, float y, float r, int i)
    {
        content.push_back(ContentItem{x, y, r, i});
        push(0, &content.back());
        max_span = std::max(max_span, r);    
    }

    
private:

    std::array<LightQuadTreeNode, static_cast<size_t>(N * 4 * 2)> nodes;
    int next_placed_node = 0;
    float max_span = 0;
    std::list<ContentItem> content;
        

    int get_next_node(const int pos, const float x, const float y)
    {
        auto sub = nodes[pos].quadrant.divide();

        if (nodes[pos].first_leaf == -1)
        {
            for (int i = 0; i < 4; ++i)
            {
                if (i == 0)
                    nodes[pos].first_leaf = create_new_node(sub[i]);
                else
                    create_new_node(sub[i]);
            }
        }
        
        for (int i = 0; i < 4; ++i)
            if (sub[i].contains(x, y))
                return nodes[pos].first_leaf + i;
        
        return -1;
    }

    void push(const int pos, const ContentItem* item)
    {
        if (!nodes[pos].content && nodes[pos].first_leaf == -1)
        {
            nodes[pos].content = item;
            return;
        }
        
        if (nodes[pos].content)
        {            
            const ContentItem* closest = nodes[pos].content;
            nodes[pos].content = nullptr;
            push(get_next_node(pos, closest->x, closest->y), closest);
        }

        push(get_next_node(pos, item->x, item->y), item);
    }

    void query_range(std::vector<int>& collection, const int pos, float x, float y, const float r) const
    {
        query_range(
            collection,
            pos,
            Quad(
                static_cast<decltype(Quad::l)>(x - r),
                static_cast<decltype(Quad::l)>(y - r),
                static_cast<decltype(Quad::l)>(x + r),
                static_cast<decltype(Quad::l)>(y + r))
        );
    }

    void query_range(std::vector<int>& collection, const int pos, const Quad & loc) const
    {
        if (nodes[pos].content)
        {
            collection.push_back(nodes[pos].content->i);
            return;
        }

        if (nodes[pos].first_leaf == -1)
            return;
        
        Quad::Subdivision sub = nodes[pos].quadrant.divide();
        for (int i = 0; i < 4; ++i)
            if (sub[i].intersects(loc))
                query_range(collection, nodes[pos].first_leaf + i, loc);
    }

    int create_new_node(const Quad & q)
    {
        nodes[next_placed_node] = LightQuadTreeNode(q);
        next_placed_node++;
        return next_placed_node - 1;
        // nodes.emplace_back(LazyQuadTreeNodeOnVector(q));
        // return static_cast<int>(nodes.size() - 1);
    }
};
