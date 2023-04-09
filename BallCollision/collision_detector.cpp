
#include "collision_detector.h"

#include <cassert>
#include <unordered_set>

#include "constants.h"
#include "profiler.h"

QuadTree::QuadTree(const std::vector<std::shared_ptr<Physical>> & objects) :
    quad_tree_root({0, 0, WINDOW_X, WINDOW_X}),
    max_span(calc_max_object_span(objects))
{
    for (const auto & ball : objects)
    {
        // if (ball->p.x < 0 || ball->p.x > WINDOW_X || ball->p.y < 0 || ball->p.y > WINDOW_Y)
        // {
        //     continue;
        // }
        
        // quad_tree_root.push(ball);
    }
    
    // assert(objects.size() >= quad_tree_root.count_items());
}

size_t proxy_reserve = 0;

void QuadTree::detect_collisions(
    const Physical* that,
    std::vector<std::pair<uint32_t, uint32_t>>& colliding_pairs,
    int* collision_test_counter
) 
{
    std::vector<const Physical*> proxy;
    // proxy.reserve(proxy_reserve);
    
    quad_tree_root.collect_proxy(that->p, max_span * 2, proxy);

    proxy_reserve = (proxy_reserve * 2 + proxy.size()) / 3;
    
    for (const auto & other : proxy)
    {
        if (that->id != other->id)
        {
            if (collision_test_counter != nullptr)
                (*collision_test_counter)++;
            
            if (that->is_touching(other))
            {
                colliding_pairs.push_back({ that->id, other->id });
            }    
        }
    }

    quad_tree_root.push(that);
}

int LightQuadTree::create_new_node(const Quadrant & q)
{
    nodes[next_placed_node] = LightQuadTreeNode(q);
    next_placed_node++;
    return next_placed_node - 1;
    // nodes.emplace_back(LazyQuadTreeNodeOnVector(q));
    // return static_cast<int>(nodes.size() - 1);
}

LightQuadTree::LightQuadTree(const std::vector<std::shared_ptr<Physical>>& objects)
: max_span(calc_max_object_span(objects))
{
    // nodes.reserve(objects.size());
    create_new_node({0, 0, WINDOW_X, WINDOW_X});
}

void LightQuadTree::detect_collisions(
    const Physical* that,
    std::vector<std::pair<uint32_t, uint32_t>>& colliding_pairs,
    int* collision_test_counter)
{
    std::vector<const Physical*> proxy;
    // proxy.reserve(proxy_reserve);
    
    collect_proxy(0, that->p, max_span * 2, proxy);

    proxy_reserve = (proxy_reserve * 2 + proxy.size()) / 3;
    
    for (const auto & other : proxy)
    {
        if (that->id != other->id)
        {
            if (collision_test_counter != nullptr)
                (*collision_test_counter)++;
            
            if (that->is_touching(other))
            {
                colliding_pairs.push_back({ that->id, other->id });
            }    
        }
    }

    push(0, that);
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

LazyQuadTreeNode* LazyQuadTreeNode::get_next_node(const sf::Vector2f loc)
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

void LazyQuadTreeNode::push(const Physical* item)
{
    {
        if (!content && !has_leaves)
        {
            content = item;
            return;
        }
    }
    
    if (content)
    {            
        const Physical* closest = content;
        content = nullptr;
        get_next_node(closest->p)->push(closest);
    }

    get_next_node(item->p)->push(item);
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

void LazyQuadTreeNode::collect_proxy(const sf::Vector2f & loc, const float R, std::vector<const Physical*>& collection) const
{
    collect_proxy(Quadrant{
        loc.x - R,
        loc.y - R,
        loc.x + R,
        loc.y + R},
        
    collection);
}


void LazyQuadTreeNode::collect_proxy(const Quadrant & loc, std::vector<const Physical*>& collection) const
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



LightQuadTreeNode::LightQuadTreeNode(const Quadrant & q_) :
quadrant(q_)
{
    assert(quadrant.l < quadrant.r && quadrant.t < quadrant.b);
}

int LightQuadTree::get_next_node(const int pos, const sf::Vector2f loc)
{
    float loc_x = loc.x;
    float loc_y = loc.y;

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
        if (sub[i].contains(loc_x, loc_y))
            return nodes[pos].first_leaf + i;
    
    return -1;
}

void LightQuadTree::push(const int pos, const Physical* item)
{
    if (!nodes[pos].content && nodes[pos].first_leaf == -1)
    {
        nodes[pos].content = item;
        return;
    }
    
    if (nodes[pos].content)
    {            
        const Physical* closest = nodes[pos].content;
        nodes[pos].content = nullptr;
        push(get_next_node(pos, closest->p), closest);
    }

    push(get_next_node(pos, item->p), item);
}

void LightQuadTree::collect_proxy(const int pos, const sf::Vector2f & loc, const float R, std::vector<const Physical*>& collection) const
{
    collect_proxy(pos,
                  Quadrant{
                      loc.x - R,
                      loc.y - R,
                      loc.x + R,
                      loc.y + R}, collection);
}


void LightQuadTree::collect_proxy(const int pos, const Quadrant & loc, std::vector<const Physical*>& collection) const
{
    if (nodes[pos].content)
    {
        collection.push_back(nodes[pos].content);
        return;
    }

    if (nodes[pos].first_leaf == -1)
        return;
    
    Subdivision sub = nodes[pos].quadrant.divide();
    for (int i = 0; i < 4; ++i)
    {
        Quadrant intersection = {};
        if (sub[i].overlap(loc, intersection))
            collect_proxy(nodes[pos].first_leaf + i, intersection, collection);
    }
}
