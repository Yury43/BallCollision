#include "QuadTreeLight.hpp"

#include "profiler.h"


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

void LightQuadTree::push(const int pos, const Collidable* item)
{
    if (!nodes[pos].content && nodes[pos].first_leaf == -1)
    {
        nodes[pos].content = item;
        return;
    }
    
    if (nodes[pos].content)
    {            
        const Collidable* closest = nodes[pos].content;
        nodes[pos].content = nullptr;
        push(get_next_node(pos, closest->p), closest);
    }

    push(get_next_node(pos, item->p), item);
}

void LightQuadTree::query_range(const int pos, const sf::Vector2f & loc, const float R, std::vector<const Collidable*>& collection) const
{
    query_range(pos,
        Quad(
            static_cast<decltype(Quad::l)>(loc.x - R),
            static_cast<decltype(Quad::l)>(loc.y - R),
            static_cast<decltype(Quad::l)>(loc.x + R),
            static_cast<decltype(Quad::l)>(loc.y + R))
        , collection
    );
}


void LightQuadTree::query_range(const int pos, const Quad & loc, std::vector<const Collidable*>& collection) const
{
    if (nodes[pos].content)
    {
        collection.push_back(nodes[pos].content);
        return;
    }

    if (nodes[pos].first_leaf == -1)
        return;
    
    Quad::Subdivision sub = nodes[pos].quadrant.divide();
    for (int i = 0; i < 4; ++i)
        if (sub[i].intersects(loc))
            query_range(nodes[pos].first_leaf + i, loc, collection);
}

int LightQuadTree::create_new_node(const Quad & q)
{
    nodes[next_placed_node] = LightQuadTreeNode(q);
    next_placed_node++;
    return next_placed_node - 1;
    // nodes.emplace_back(LazyQuadTreeNodeOnVector(q));
    // return static_cast<int>(nodes.size() - 1);
}

LightQuadTree::LightQuadTree(float l, float t, float r, float b)
{
    // nodes.reserve(objects.size());
    create_new_node(Quad{l, t, r, b});
}

void LightQuadTree::detect_collisions(
    const std::vector<std::shared_ptr<Collidable>> & agents,
    const int index,
    std::vector<std::pair<uint32_t, uint32_t>>& colliding_pairs,
    int* collision_test_counter)
{
    PROFILE_NAMED("detect_collisions");
    const Collidable* item = agents[index].get();
    std::vector<const Collidable*> proxy;
    // proxy.reserve(proxy_reserve);
    float thatSpan = item->span();
    // sf::Vector2f c = that->p - sf::Vector2f{ 1, 1 };
    sf::Vector2f c = item->p;
    query_range(0, c, thatSpan + max_span + 1, proxy);
    max_span = std::max(max_span, thatSpan);

    for (const auto & other : proxy)
    {
        if (item->id != other->id)
        {
            // if (collision_test_counter != nullptr)
            // (*collision_test_counter)++;
            
            if (item->is_touching(other))
            {
                colliding_pairs.push_back({std::min(item->id, other->id), std::max(item->id, other->id)});
            }    
        }
    }

    push(0, item);
}
