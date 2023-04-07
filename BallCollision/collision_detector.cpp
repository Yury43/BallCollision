
#include "collision_detector.h"

#include <cassert>
#include <unordered_set>

#include "constants.h"

OfflineQuadTree::OfflineQuadTree(const std::vector<std::shared_ptr<Ball>> & balls) :
    quad_tree_root({0, 0, WINDOW_X, WINDOW_X}),
    max_span(calc_max_object_span(balls))
{
    for (const auto & ball : balls)
    {
        if (ball->p.x < 0 || ball->p.x > WINDOW_X || ball->p.y < 0 || ball->p.y > WINDOW_Y)
        {
            continue;
        }
        
        quad_tree_root.push(ball);
    }
    
    assert(balls.size() >= quad_tree_root.count_items());
}
    
void OfflineQuadTree::detect_collisions(
    const std::shared_ptr<Ball> & ball,
    std::vector<std::pair<int, int>> & colliding_pairs,
    int* collision_test_counter
) 
{
    std::vector<std::shared_ptr<Ball>> proxy;
    quad_tree_root.collect_proxy(ball->p, max_span * 2, proxy);
        
    // std::unordered_set<uint32_t> ids;
    for (const auto & other : proxy)
    {
        // ids.insert(other->id);
        if (ball->id != other->id)
        {
            if (collision_test_counter != nullptr)
                (*collision_test_counter)++;
            
            if (ball->is_touching(other.get()))
            {
                colliding_pairs.push_back({ ball->id, other->id });
            }    
        }
    }
        
    // std::cout << proxy.size() << "/" << ids.size() << " ";
}

OnlineQuadTree::OnlineQuadTree() : quad_tree_root({0, 0, WINDOW_X, WINDOW_X})
{
    
}

void OnlineQuadTree::detect_collisions(
    const std::shared_ptr<Ball> & ball,
    std::vector<std::pair<int, int>> & colliding_pairs,
    int* collision_test_counter
    ) 
{
    if (ball->p.x < 0 || ball->p.x > WINDOW_X || ball->p.y < 0 || ball->p.y > WINDOW_Y)
    {
        return;
    }
    
    std::shared_ptr<Physical> closest = quad_tree_root.push(ball);
    if (closest)
    {
        if (collision_test_counter != nullptr)
            (*collision_test_counter)++;
        
        if (ball->is_touching(closest.get()))
        {
            colliding_pairs.push_back({ ball->id, closest->id });
        }
    }
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


LazyQuadTreeNode::LazyQuadTreeNode(const Quadrant & q_) : quadrant(q_), subquadrants(q_.divide())
{
    assert(quadrant.l < quadrant.r);
    assert(quadrant.t < quadrant.b);
}

std::shared_ptr<Ball> LazyQuadTreeNode::push(const std::shared_ptr<Ball> & item)
{

    {
        bool is_a_leaf = is_leaf();
        assert(!content || !is_a_leaf);

        if (!content && !is_a_leaf)
        {
            content = item;
            return nullptr;
        }
    }
    
    auto find_next_node = [=](const sf::Vector2f loc) -> std::shared_ptr<LazyQuadTreeNode>
    {
        float loc_x = loc.x;
        float loc_y = loc.y;

        for (int i = 0; i < 4; ++i)
        {
            if (subquadrants[i].contains(loc_x, loc_y))
            {
                if (!leaves[i]) leaves[i] = std::make_shared<LazyQuadTreeNode>(subquadrants[i]);
                    return leaves[i];
            }
        }
    };
    
    if (content)
    {            
        std::shared_ptr<Ball> closest = content;    
        content.reset();
        
        find_next_node(closest->p)->push(closest);
        find_next_node(item->p)->push(item);
        
        return closest;
    }
    else
    {
        return find_next_node(item->p)->push(item);
    }
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

bool LazyQuadTreeNode::is_leaf() const
{
    for (const auto & leaf : leaves)
        if (leaf)
            return true;
    
    return false;
}

void LazyQuadTreeNode::collect_proxy(const sf::Vector2f & loc, const float R, std::vector<std::shared_ptr<Ball>> & collection) const
{
    collect_proxy(Quadrant{
        loc.x - R,
        loc.y - R,
        loc.x + R,
        loc.y + R},
    collection);
}


void LazyQuadTreeNode::collect_proxy(const Quadrant & loc, std::vector<std::shared_ptr<Ball>>& collection) const
{
    if (content)
    {
        collection.push_back(content);
        return;
    }
    
    for (int i = 0; i < 4; ++i)
    {
        Quadrant intersection;
        if (subquadrants[i].overlap(loc, intersection))
        {
            if (leaves[i]) leaves[i]->collect_proxy(intersection, collection);
        }
    }
}
