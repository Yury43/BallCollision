
#include "collision_detector.h"

#include <cassert>
#include <random>
#include <unordered_set>

#include "ball.h"
#include "constants.h"
#include "profiler.h"
#include "quadtree/Quadtree.h"
#include <functional>

QuadTree::QuadTree() :
    root({0, 0, WINDOW_X, WINDOW_X})
{

}

void QuadTree::detect_collisions(
    const Physical* that,
    const int index,
    std::vector<std::pair<uint32_t, uint32_t>>& colliding_pairs,
    int* collision_test_counter
) 
{
    std::vector<const Physical*> proxy;
    // proxy.reserve(proxy_reserve);
    // std::cout << "items: " << root.count_items() << "\t" << "nodes: " << root.count_nodes() << std::endl;
    float that_span = that->span();
    root.query_range(that->p, max_span + that_span + 2, proxy);
    
    for (const auto & other : proxy)
    {
        if (that->id != other->id)
        {
            if (collision_test_counter != nullptr)
                (*collision_test_counter)++;
            
            if (that->is_touching(other))
            {
                colliding_pairs.push_back({ std::min(that->id, other->id) , std::max(that->id, other->id)});
            }    
        }
        else
        {
            assert(false);
        }
    }

    LazyQuadTreeNode* node = root.push(that);
    // const Physical* closest2 = root.find_closest(that->p);
    // assert(closest2->id == that->id);

    max_span = std::max(max_span, that_span);
}

int LightQuadTree::create_new_node(const Quad & q)
{
    nodes[next_placed_node] = LightQuadTreeNode(q);
    next_placed_node++;
    return next_placed_node - 1;
    // nodes.emplace_back(LazyQuadTreeNodeOnVector(q));
    // return static_cast<int>(nodes.size() - 1);
}

LightQuadTree::LightQuadTree()
{
    // nodes.reserve(objects.size());
    create_new_node({-MAX_R, -MAX_R, WINDOW_X + MAX_R, WINDOW_Y + MAX_R});
}

void LightQuadTree::detect_collisions(
    const Physical* that,
    const int index,
    std::vector<std::pair<uint32_t, uint32_t>>& colliding_pairs,
    int* collision_test_counter)
{
    std::vector<const Physical*> proxy;
    // proxy.reserve(proxy_reserve);
    float thatSpan = that->span();
    // sf::Vector2f c = that->p - sf::Vector2f{ 1, 1 };
    sf::Vector2f c = that->p;
    query_range(0, c, thatSpan + max_span + 1, proxy);
    max_span = std::max(max_span, thatSpan);

    for (const auto & other : proxy)
    {
        if (that->id != other->id)
        {
            // if (collision_test_counter != nullptr)
                // (*collision_test_counter)++;
            
            if (that->is_touching(other))
            {
                colliding_pairs.push_back({std::min(that->id, other->id), std::max(that->id, other->id)});
            }    
        }
    }

    push(0, that);
}

std::vector<Node> generateRandomNodes(std::size_t n)
{
    auto generator = std::default_random_engine();
    auto originDistribution = std::uniform_real_distribution(0.0f, 1.0f);
    auto sizeDistribution = std::uniform_real_distribution(0.0f, 0.01f);
    auto nodes = std::vector<Node>(n);
    for (auto i = std::size_t(0); i < n; ++i)
    {
        nodes[i].box.left = originDistribution(generator);
        nodes[i].box.top = originDistribution(generator);
        nodes[i].box.width = std::min(1.0f - nodes[i].box.left, sizeDistribution(generator));
        nodes[i].box.height = std::min(1.0f - nodes[i].box.top, sizeDistribution(generator));
        nodes[i].id = i;
    }
    return nodes;
}


std::vector<std::pair<Node*, Node*>> computeIntersections(std::vector<Node>& nodes, const std::vector<bool>& removed)
{
    auto intersections = std::vector<std::pair<Node*, Node*>>();
    for (auto i = std::size_t(0); i < nodes.size(); ++i)
    {
        if (removed.size() == 0 || !removed[i])
        {
            for (auto j = std::size_t(0); j < i; ++j)
            {
                if (removed.size() == 0 || !removed[j])
                {
                    if (nodes[i].box.intersects(nodes[j].box))
                        intersections.emplace_back(&nodes[i], &nodes[j]);
                }
            }
        }
    }
    return intersections;
}

void checkIntersections(std::vector<Node*> nodes1, std::vector<Node*> nodes2)
{
    assert(nodes1.size() == nodes2.size());
    std::sort(std::begin(nodes1), std::end(nodes1));
    std::sort(std::begin(nodes2), std::end(nodes2));
    for (auto i = std::size_t(0); i < nodes1.size(); ++i)
        assert(nodes1[i] == nodes2[i]);
}


QuadTreePvigier::QuadTreePvigier(const std::vector<std::shared_ptr<Physical>>& objects)
{
    float rad = 0;

    getBox = [](Node* node)
    {
        return node->box;
    };

    box = quadtree::Box(-rad - 1, -rad - 1, WINDOW_X + rad + 2, WINDOW_X + rad + 2);
    t = std::make_unique<quadtree::Quadtree<Node*, decltype(getBox)>>(box, getBox);
    nodes.resize(objects.size());
    
    for (size_t i = 0; i < objects.size(); ++i)
    {
        Ball* b = dynamic_cast<Ball*>(&*objects[i]);
        if (b == nullptr)
            continue;
        
        nodes[i].box.left = b->p.x - b->R;
        nodes[i].box.top = b->p.y - b->R;
        nodes[i].box.width = b->R * 2;
        nodes[i].box.height = b->R * 2;
        nodes[i].id = b->id;
    }
    
    for (auto& node : nodes)
    {
        if (box.contains(node.box))
        {
            t->add(&node);
        }
    }
}

void QuadTreePvigier::detect_collisions(const Physical* ball, const int index, std::vector<std::pair<uint32_t, uint32_t>>& colliding_pairs, int* collision_test_counter)
{
    if (box.contains(nodes[index].box))
    {
        std::vector<Node*> intersections = t->query(nodes[index].box);
        for (const auto & it : intersections)
        {
            colliding_pairs.push_back({ball->id, it->id});
        }    
    }
}


// subquadrants are not initialized on purpose 
LazyQuadTreeNode::LazyQuadTreeNode(const Quad & q_) :
quadrant(q_)
, subquadrants(q_.divide())
{
    assert(quadrant.l < quadrant.r && quadrant.t < quadrant.b);
}

LazyQuadTreeNode* LazyQuadTreeNode::get_next_node(const sf::Vector2f loc)
{
    float loc_x = loc.x;
    float loc_y = loc.y;

    // subquadrants = quadrant.divide();
    
    for (int i = 0; i < 4; ++i)
    {
        if (subquadrants[i].contains(loc_x, loc_y))
        {
            if (!leaves[i])
            {
                leaves[i] = std::make_unique<LazyQuadTreeNode>(subquadrants[i]);
                got_leaves = true;
            }
            
            return leaves[i].get();
        }
    }

    assert(false);
    return nullptr;
}

void LazyQuadTreeNode::add_content(const Physical* item)
{
    // content.push_back(item);
    content = item;
}

bool LazyQuadTreeNode::got_content() const
{
    return content != nullptr;
    // return !content.empty();
}

size_t LazyQuadTreeNode::content_size() const
{
    return content != nullptr;
    // return content.size();
}

void LazyQuadTreeNode::collect_content(std::vector<const Physical*>& collection) const
{
    // collection.insert(collection.end(), content.begin(), content.end());
    collection.push_back(content);
}

void LazyQuadTreeNode::push_current_content()
{
    // auto closest = std::move(content);
    // for (const auto & c : closest)
    // {
    //     get_next_node(c->p)->push(c);
    // }
    auto closest = content;
    content = nullptr;
    get_next_node(closest->p)->push(closest);
}

LazyQuadTreeNode* LazyQuadTreeNode::push(const Physical* item)
{
    // std::cout << "push " << item->id << "\tat " << item->p << "\tinto " << quadrant.l << "\t" << quadrant.t << "\t" << quadrant.r << "\t" << quadrant.b << std::endl;
    if (!got_content())
    {
        if (got_leaves)
        {
            return get_next_node(item->p)->push(item);
        }
        else
        {
            add_content(item);
            return this;
        }
    }
    else
    {
        // if (quadrant.r - quadrant.l > 2)
        {
            push_current_content();
            return get_next_node(item->p)->push(item);
        }
        // else
        // {
        //     content.push_back(item);
        // }
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



void LazyQuadTreeNode::query_range(const sf::Vector2f & loc, const float R, std::vector<const Physical*>& collection) const
{
    query_range(Quad{
        static_cast<decltype(Quad::l)>(loc.x - R),
        static_cast<decltype(Quad::l)>(loc.y - R),
        static_cast<decltype(Quad::l)>(loc.x + R),
        static_cast<decltype(Quad::l)>(loc.y + R)}
        ,
    collection);
}



void LazyQuadTreeNode::query_range(const Quad & loc, std::vector<const Physical*>& collection) const
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

const Physical* LazyQuadTreeNode::find_closest(const sf::Vector2f & loc)
{
    if (got_content())
    {
        return content;
    }
    else if (got_leaves)
    {
        for (int i = 0; i < 4; ++i)
        {
            if (subquadrants[i].contains(loc.x, loc.y))
            {
                if (leaves[i])
                {
                    return leaves[i]->find_closest(loc);
                }
            }
        }
    }
    // assert(false);
    return nullptr;
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

void LightQuadTree::query_range(const int pos, const sf::Vector2f & loc, const float R, std::vector<const Physical*>& collection) const
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


void LightQuadTree::query_range(const int pos, const Quad & loc, std::vector<const Physical*>& collection) const
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
