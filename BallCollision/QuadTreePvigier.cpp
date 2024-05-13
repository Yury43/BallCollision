#include "QuadTreePvigier.h"

#include <random>

#include "ball.h"
#include "constants.h"


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

void QuadTreePvigier::detect_collisions(
    const std::vector<std::shared_ptr<Physical>> & agents,
    const int index,
    std::vector<std::pair<uint32_t, uint32_t>>& colliding_pairs,
    int* collision_test_counter)
{
    // PROFILE_NAMED("detect_collisions");
    const Physical* ball = agents[index].get();
    if (box.contains(nodes[index].box))
    {
        std::vector<Node*> intersections = t->query(nodes[index].box);
        for (const auto & it : intersections)
        {
            if (ball->id != it->id)
            {
                colliding_pairs.push_back({std::min<>(ball->id, it->id), std::max(ball->id, it->id)});
            }
        }    
    }
}
