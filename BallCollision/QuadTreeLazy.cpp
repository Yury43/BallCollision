#include "QuadTreeLazy.hpp"

#include <cassert>


LazyQuadTree::LazyQuadTree(int l, int t, int r, int b) :
    root({l, t, r, b})
{

}

void LazyQuadTree::detect_collisions(
    const std::vector<std::shared_ptr<Collidable>> & agents,
    const int index,
    std::vector<std::pair<uint32_t, uint32_t>>& colliding_pairs,
    int* collision_test_counter
) 
{
    // PROFILE_NAMED("detect_collisions");
    std::vector<const Collidable*> proxy;
    // proxy.reserve(proxy_reserve);
    // std::cout << "items: " << root.count_items() << "\t" << "nodes: " << root.count_nodes() << std::endl;
    const Collidable* item = agents[index].get();
    float that_span = item->span();
    root.query_range(item->p.x, item->p.y, max_span + that_span, proxy);
    
    for (const auto & other : proxy)
    {
        if (item->id != other->id)
        {
            if (collision_test_counter != nullptr)
                (*collision_test_counter)++;
            
            if (item->is_touching(other))
            {
                // colliding_pairs.push_back({ std::min(that->id, other->id) , std::max(that->id, other->id)});
                colliding_pairs.push_back({item->id, other->id});
            }    
        }
    }


}

void LazyQuadTree::add(const Collidable* c)
{
    root.push(c);
    max_span = std::max(max_span, c->span());
}

// subquadrants are not initialized on purpose 
LazyQuadTreeNode::LazyQuadTreeNode(const Quad & q_) :
quadrant(q_)
, subquadrants(q_.divide())
{
    assert(quadrant.l < quadrant.r && quadrant.t < quadrant.b);
}

LazyQuadTreeNode* LazyQuadTreeNode::get_next_node(const float x, const float y)
{

    // subquadrants = quadrant.divide();
    
    for (int i = 0; i < 4; ++i)
    {
        if (subquadrants[i].contains(x, y))
        {
            if (!leaves[i])
            {
                leaves[i] = std::make_unique<LazyQuadTreeNode>(subquadrants[i]);
                // leaves[i] = new LazyQuadTreeNode(subquadrants[i]);
                got_leaves = true;
            }
            
            // return leaves[i];
            return leaves[i].get();
        }
    }

    assert(false);
    return nullptr;
}

void LazyQuadTreeNode::add_content(const Collidable* item)
{
    content.push_back(item);
    // content = item;
}

bool LazyQuadTreeNode::got_content() const
{
    // return content != nullptr;
    return !content.empty();
}

size_t LazyQuadTreeNode::content_size() const
{
    // return content != nullptr;
    return content.size();
}

void LazyQuadTreeNode::collect_content(std::vector<const Collidable*>& collection) const
{
    collection.insert(collection.end(), content.begin(), content.end());
    // collection.push_back(content);
}

void LazyQuadTreeNode::push_current_content()
{
    auto closest = std::move(content);
    for (const auto & c : closest)
    {
        get_next_node(c->p.x, c->p.y)->push(c);
    }
    // auto closest = content;
    // content = nullptr;
    // get_next_node(closest->p)->push(closest);
}

LazyQuadTreeNode* LazyQuadTreeNode::push(const Collidable* item)
{
    // std::cout << "push " << item->id << "\tat " << item->p << "\tinto " << quadrant.l << "\t" << quadrant.t << "\t" << quadrant.r << "\t" << quadrant.b << std::endl;
    if (!got_content())
    {
        if (got_leaves)
        {
            return get_next_node(item->p.x, item->p.x)->push(item);
        }
        else
        {
            add_content(item);
            return this;
        }
    }
    else
    {
        if (quadrant.r - quadrant.l > 3)
        {
            push_current_content();
            return get_next_node(item->p.x, item->p.y)->push(item);
        }
        else
        {
            content.push_back(item);
        }
    }
    return nullptr;
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
    
    if (got_content())
    {
        size += content_size();
    }
    
    for (const auto & leaf : leaves)
        if (leaf)
            size += leaf->count_items();
            
    return size;
}



void LazyQuadTreeNode::query_range(const float cx, const float cy, const float R, std::vector<const Collidable*>& collection) const
{
    query_range(Quad{
        static_cast<decltype(Quad::l)>(cx - R),
        static_cast<decltype(Quad::l)>(cy - R),
        static_cast<decltype(Quad::l)>(cx + R),
        static_cast<decltype(Quad::l)>(cy + R)}
        ,
    collection);
}



void LazyQuadTreeNode::query_range(const Quad & loc, std::vector<const Collidable*>& collection) const
{
    if (got_content())
    {
        collect_content(collection);
        return;
    }
    // else if (!got_leaves)
    {
        for (int i = 0; i < 4; ++i)
        {
            if (subquadrants[i].intersects(loc))
                if (leaves[i])
                    leaves[i]->query_range(loc, collection);
        }  
    }
}

