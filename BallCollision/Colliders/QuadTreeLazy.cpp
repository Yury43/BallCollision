#include "QuadTreeLazy.hpp"

#include <cassert>
#include <memory>


LazyQuadTree::LazyQuadTree(int l, int t, int r, int b) :
    root({l, t, r, b})
{

}

void LazyQuadTree::add(const float x, const float y, const float r, const int i)
{
    content.push_back(ContentItem{x, y, r, i});
    
    root.push(&content.back());
    max_span = std::max(max_span, r);
}

void LazyQuadTree::detect_collisions(std::vector<int>& proxy, const float x, const float y, const float r, const int id) 
{
    root.query_range(proxy, x, y, max_span + r);
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

void LazyQuadTreeNode::add_content(const ContentItem* item)
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

void LazyQuadTreeNode::collect_content(std::vector<int>& collection) const
{
    for (const auto & item : content)
        collection.push_back(item->i);
}

void LazyQuadTreeNode::push_current_content()
{
    auto closest = std::move(content);
    for (const auto & c : closest)
    {
        get_next_node(c->x, c->y)->push(c);
    }
    // auto closest = content;
    // content = nullptr;
    // get_next_node(closest->p)->push(closest);
}

LazyQuadTreeNode* LazyQuadTreeNode::push(const ContentItem* item)
{
    // std::cout << "push " << item->id << "\tat " << item->p << "\tinto " << quadrant.l << "\t" << quadrant.t << "\t" << quadrant.r << "\t" << quadrant.b << std::endl;
    if (!got_content())
    {
        if (got_leaves)
        {
            return get_next_node(item->x, item->y)->push(item);
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
            return get_next_node(item->x, item->y)->push(item);
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
    {
        if (leaf)
        {
            size += leaf->count_nodes();
        }
    }
    
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
    {
        if (leaf)
        {
            size += leaf->count_items();
        }
    }
            
    return size;
}



void LazyQuadTreeNode::query_range(std::vector<int>& collection, const float x, const float y, const float r) const
{
    query_range(
        collection,
        Quad{
            static_cast<decltype(Quad::l)>(x - r),
            static_cast<decltype(Quad::t)>(y - r),
            static_cast<decltype(Quad::r)>(x + r),
            static_cast<decltype(Quad::b)>(y + r)}
    );
}



void LazyQuadTreeNode::query_range(std::vector<int>& collection, const Quad & loc) const
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
            {
                if (leaves[i])
                {
                    leaves[i]->query_range(collection, loc);
                }
            }
        }  
    }
}

