#pragma once
#include <memory>
#include <array>
#include <iostream>

#include "constants.h"
#include "physicals.h"


class CollisionDetector
{
protected:
    
    template<typename T>
    float calc_max_object_span(const std::vector<std::shared_ptr<T>>& objects)
    {
        return objects.empty() ? 0 :
        (*std::max_element(objects.begin(), objects.end(), [](const auto& a, const auto& b) { return a->R < b->R; }))->R;
    }
    
public:
    
    virtual ~CollisionDetector() = default;
    
    virtual void detect_collisions(
        const std::shared_ptr<Ball> & ball,
        std::vector<std::pair<int, int>> & colliding_pairs,
        int* collision_test_counter = nullptr
    ) = 0;
    
};


template<class T>
class BinGrid : public CollisionDetector
{
public:
    explicit BinGrid(std::vector<std::shared_ptr<T>>& objects) :
        bin_size(calc_max_object_span(objects) * 2),
        n_bins_x( static_cast<int>(std::ceil(WINDOW_X / bin_size)) + 2),
        n_bins_y( static_cast<int>(std::ceil(WINDOW_Y / bin_size)) + 2)
    {
        
        bins = std::vector<std::vector<Bin>>(n_bins_x, std::vector<Bin>(n_bins_y));
    
        // Place balls in bins, delete those that escaped 
        {
            auto oit = objects.begin();
            while (oit != objects.end())
            {
                const int bin_x = find_bin_x(*oit);
                const int bin_y = find_bin_y(*oit);

                if (out_of_range(bin_x, bin_y))
                {
                    std::cout << "Object " << (*oit)->id << " " << (*oit)->p << " escaped to [" << bin_x << ", " << bin_y << "] !" << std::endl;
                    oit = objects.erase(oit);
                }
                else
                {
                    bins[bin_x][bin_y].push_back(*oit);
                    ++oit;
                }
            }
        }
    }

    ~BinGrid() override = default;
    
    void detect_collisions(
        const std::shared_ptr<T> & that,
        std::vector<std::pair<int, int>> & colliding_pairs,
        int* collision_test_counter = nullptr
        
    ) override 
    {
        int bin_x = find_bin_x(that);
        int bin_y = find_bin_y(that);

        for (int i = 0; i < 2; ++i)
        {
            for (int j = 0; j < 2; ++j)
            {
                if (out_of_range(bin_x + i, bin_y + j))
                    continue;

                for (auto& other : bins[bin_x + i][bin_y + j])
                {
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
            }
        }
    }

private:
    using Bin = std::vector<std::shared_ptr<T>>;
    
    const float bin_size;
    const int n_bins_x;
    const int n_bins_y;

    std::vector<std::vector<Bin>> bins;
    
    int find_bin_x(const std::shared_ptr<T> & pp) const  
    {
        return static_cast<int>(std::round(pp->p.x / bin_size)) + 1; 
    }
    
    int find_bin_y(const std::shared_ptr<T> & pp) const 
    {
        return static_cast<int>(std::round(pp->p.y / bin_size)) + 1; 
    }

    bool out_of_range(const int bin_x, const int bin_y) const
    {
        return bin_x < 0 or bin_x >= n_bins_x or bin_y < 0 or bin_y >= n_bins_y;
    }
};

struct Quadrant;
using Subdivision = std::array<Quadrant, 4>;

struct Quadrant
{
    float l, t, r, b;

    Quadrant() = default;
    
    bool overlap(const Quadrant & other, Quadrant & intersection) const;
    Subdivision divide() const; 
    bool contains(float x, float y) const; 
};

class LazyQuadTreeNode
{
public:
    LazyQuadTreeNode(const Quadrant & q);

    std::shared_ptr<Ball> push(const std::shared_ptr<Ball> & item);

    size_t count_nodes() const ;

    size_t count_items() const ;

    bool is_leaf() const;
    
    void collect_proxy(const sf::Vector2f & loc, const float R, std::vector<std::shared_ptr<Ball>> & collection) const;

    void collect_proxy(const Quadrant & loc, std::vector<std::shared_ptr<Ball>>& collection) const;
    
private:

    Quadrant quadrant;
    Subdivision subquadrants;

    std::array<std::shared_ptr<LazyQuadTreeNode>, 4> leaves;

    std::shared_ptr<Ball> content;
};


class OfflineQuadTree : public CollisionDetector
{
public:
    explicit  OfflineQuadTree(const std::vector<std::shared_ptr<Ball>> & balls) ;

    ~OfflineQuadTree() override = default;
    
    void detect_collisions(
        const std::shared_ptr<Ball> & ball,
        std::vector<std::pair<int, int>> & colliding_pairs,
        int* collision_test_counter = nullptr
    ) override;
    
private:
    
    LazyQuadTreeNode quad_tree_root;
    const float max_span;
};


class OnlineQuadTree : public CollisionDetector
{
public:

    OnlineQuadTree();

    ~OnlineQuadTree() override = default;
    
    void detect_collisions(
        const std::shared_ptr<Ball> & ball,
        std::vector<std::pair<int, int>> & colliding_pairs,
        int* collision_test_counter = nullptr
        ) override;
    
private:
    LazyQuadTreeNode quad_tree_root;
};

