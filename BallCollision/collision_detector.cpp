
#include "collision_detector.h"

#include <cassert>
#include <unordered_set>

#include "constants.h"
#include "profiler.h"

QuadTree::QuadTree(const std::vector<std::shared_ptr<Physical>> & objects) :
    quad_tree_root({0, 0, WINDOW_X, WINDOW_X}),
    max_span(calc_max_object_span(objects))
{
    PROFILE();
    for (const auto & ball : objects)
    {
        // if (ball->p.x < 0 || ball->p.x > WINDOW_X || ball->p.y < 0 || ball->p.y > WINDOW_Y)
        // {
        //     continue;
        // }
        
        quad_tree_root.push(ball);
    }
    
    // assert(objects.size() >= quad_tree_root.count_items());
}

size_t proxy_reserve = 0;

void QuadTree::detect_collisions(
    const std::shared_ptr<Physical> & that,
    std::vector<std::pair<int, int>> & colliding_pairs,
    int* collision_test_counter
) 
{
    PROFILE();
    
    std::vector<std::shared_ptr<Physical>> proxy;
    // proxy.reserve(proxy_reserve);
    
    quad_tree_root.collect_proxy(that->p, max_span * 2, proxy);

    proxy_reserve = (proxy_reserve * 2 + proxy.size()) / 3;
    
    // std::unordered_set<uint32_t> ids;
    for (const auto & other : proxy)
    {
        // ids.insert(other->id);
        if (that->id != other->id)
        {
            if (collision_test_counter != nullptr)
                (*collision_test_counter)++;
            
            if (that->is_touching(other.get()))
            {
                colliding_pairs.push_back({ that->id, other->id });
            }    
        }
    }

    quad_tree_root.push(that);
    // std::cout << proxy.size() << "/" << ids.size() << " ";
}



bool Quadrant::overlap(const Quadrant & other, Quadrant & intersection) const 
{
    if (other.l > r || other.t > b || other.r < l || other.b < t)
    {
        return false;
    }

    intersection = {
        std::max(l, other.l),
        std::max(t, other.t),
        std::min(r, other.r),
        std::min(b, other.b),
    };
    
    return true;
}

Subdivision Quadrant::divide() const 
{
    float mx = (l + r) / 2;
    float my = (t + b) / 2;
    
    return Subdivision
    {
        Quadrant{l, t, mx, my},
        Quadrant{mx, t, r, my},
        Quadrant{mx, my, r, b},
        Quadrant{l, my, mx, b}
    };
}

bool Quadrant::contains(const float x, const float y) const 
{
    return l <= x && x <= r && t <= y && y <= b;
}


LazyQuadTreeNode::LazyQuadTreeNode(const Quadrant & q_) :
quadrant(q_)
// , subquadrants(q_.divide())
{
    assert(quadrant.l < quadrant.r && quadrant.t < quadrant.b);
}

LazyQuadTreeNode* LazyQuadTreeNode::find_next_node(const sf::Vector2f loc)
{
    float loc_x = loc.x;
    float loc_y = loc.y;

    subquadrants = quadrant.divide();
    
    for (int i = 0; i < 4; ++i)
    {
        if (subquadrants[i].contains(loc_x, loc_y))
        {
            if (!leaves[i])
            {
                leaves[i] = std::make_unique<LazyQuadTreeNode>(subquadrants[i]);
                has_leaves = true;
            }
            
            return leaves[i].get();
        }
    }
        
    return nullptr;
}

void LazyQuadTreeNode::push(const std::shared_ptr<Physical> & item)
{
    {
        // if (!content && !has_leaves())
        if (!content && !has_leaves)
        {
            content = item;
            return;
        }
    }
    
    if (content)
    {            
        std::shared_ptr<Physical> closest = std::move(content);    
        find_next_node(closest->p)->push(closest);
    }

    find_next_node(item->p)->push(item);
}

size_t LazyQuadTreeNode::count_nodes() const 
{
    size_t size = 1;

    for (const auto & leaf : leaves)
        if (leaf)
            size += leaf->count_nodes();
    
    return size;
}

size_t LazyQuadTreeNode::count_items() const 
{
    size_t size = 0;
    
    if (content) size += 1;
    for (const auto & leaf : leaves)
        if (leaf)
            size += leaf->count_items();
            
    return size;
}

// bool LazyQuadTreeNode::has_leaves() const
// {
//     return std::any_of(leaves.cbegin(), leaves.cend(), [](const std::unique_ptr<LazyQuadTreeNode> & leaf){return leaf != nullptr;});
// }

void LazyQuadTreeNode::collect_proxy(const sf::Vector2f & loc, const float R, std::vector<std::shared_ptr<Physical>> & collection) const
{
    // PROFILE();
    
    collect_proxy(Quadrant{
        loc.x - R,
        loc.y - R,
        loc.x + R,
        loc.y + R},
        
    collection);
}


void LazyQuadTreeNode::collect_proxy(const Quadrant & loc, std::vector<std::shared_ptr<Physical>>& collection) const
{
    if (content)
    {
        collection.push_back(content);
        return;
    }
    
    for (int i = 0; i < 4; ++i)
    {
        Quadrant intersection = {};
        if (subquadrants[i].overlap(loc, intersection))
        {
            if (leaves[i])
                leaves[i]->collect_proxy(intersection, collection);
        }
    }
}
