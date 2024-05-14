#include "QuadTreeAntymon.h"

#include "profiler.h"


QuadTreeAntymon::QuadTreeAntymon(int l, int t, int r, int b, int max_elements, int max_depth)
{
    qt = new Quadtree();
    qt_create(qt, l, t, r, b, max_elements, max_depth);

    
}

QuadTreeAntymon::~QuadTreeAntymon()
{
    qt_destroy(qt);
}

void QuadTreeAntymon::detect_collisions(
    const std::vector<std::shared_ptr<Collidable>> & agents,
    const int index,
    std::vector<std::pair<uint32_t, uint32_t>>& colliding_pairs,
    int* collision_test_counter)
{
    // PROFILE_NAMED("detect_collisions");
    std::vector<const Collidable*> proxy;
    // proxy.reserve(proxy_reserve);
    // std::cout << "items: " << root.count_items() << "\t" << "nodes: " << root.count_nodes() << std::endl;

    const Collidable* item = agents[index].get();
    float r = item->span();

    IntList* list = new IntList;
    il_create(list, 1);
    
    qt_query(
        qt,
        list,
        item->p.x - r - 1, 
        item->p.y - r - 1, 
        item->p.x + r + 1, 
        item->p.y + r + 1,
        index
    );

    for (int i = 0; i < il_size(list); ++i)
    {
        int id_other = il_get(list, i, 0);
        // if (item->id != id_other)
        {
            if (collision_test_counter != nullptr)
                (*collision_test_counter)++;
            
            if (item->is_touching(agents[id_other].get()))
            {
                // colliding_pairs.push_back({ std::min(that->id, other->id) , std::max(that->id, other->id)});
                colliding_pairs.push_back({item->id, agents[id_other]->id});
            }    
        }
    }
    
    il_destroy(list);

    qt_insert(
        qt,
        index,
        item->p.x - r, 
        item->p.y - r, 
        item->p.x + r, 
        item->p.y + r 
    );
}

void QuadTreeAntymon::reset_agents(const std::vector<std::shared_ptr<Collidable>> & agents)
{
    PROFILE_NAMED("reset_agents");
    for (int i = 0; i < agents.size(); ++i)
    {
        qt_remove(qt, i);
    }
    qt_cleanup(qt);
}
