#include "QuadTreeAntymon.hpp"

#include <vector>



QuadTreeAntymon::QuadTreeAntymon(int l, int t, int r, int b, int max_elements, int max_depth)
{
    qt = new Quadtree();
    qt_create(qt, l, t, r, b, max_elements, max_depth);
}

QuadTreeAntymon::~QuadTreeAntymon()
{
    qt_destroy(qt);
}

void QuadTreeAntymon::detect_collisions(std::vector<int>& proxy, const float x, const float y, const float r, const int id)
{
    IntList* list = new IntList;
    il_create(list, 1);
    
    qt_query(
        qt,
        list,
        x - r - 1, 
        y - r - 1, 
        x + r + 1, 
        y + r + 1,
        id
    );

    for (int i = 0; i < il_size(list); ++i)
    {
        int id_other = il_get(list, i, 0);
        {
            proxy.push_back(id_other);
        }    
    }
    
    il_destroy(list);

}

// void QuadTreeAntymon::reset_agents(const std::vector<std::shared_ptr<Collidable>> & agents)
// {
//     PROFILE_NAMED("reset_agents");
//     for (int i = 0; i < agents.size(); ++i)
//     {
//         qt_remove(qt, i);
//     }
//     qt_cleanup(qt);
// }

void QuadTreeAntymon::add(float x, float y, float r, int i)
{
    
    qt_insert(
        qt,
        i,
        x - r, 
        y - r, 
        x + r, 
        y + r 
    );
}
