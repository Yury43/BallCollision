#pragma once
#include <memory>
#include <array>
#include <iostream>
#include <optional>

#include "constants.h"
#include "physicals.h"


class CollisionDetector
{
public:
    
    CollisionDetector() = default;
    virtual ~CollisionDetector() = default;
    
    CollisionDetector(const CollisionDetector&) = delete;
    CollisionDetector(CollisionDetector &&) = delete;
    CollisionDetector& operator =(CollisionDetector &&) = delete;
    // CollisionDetector operator =(CollisionDetector) = delete;
    auto operator=(const CollisionDetector &) -> CollisionDetector & = delete;

    
    virtual void detect_collisions(
        const Physical* ball,
        std::vector<std::pair<uint32_t, uint32_t>>& colliding_pairs,
        int* collision_test_counter = nullptr
    ) = 0;

protected:
    
    template<typename T>
    float calc_max_object_span(const std::vector<std::shared_ptr<T>>& objects)
    {
        return objects.empty() ? 0 :
        (*std::max_element(objects.begin(), objects.end(), [](const auto& a, const auto& b) { return a->span() < b->span(); }))->span();
    }
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
        const Physical* that,
        std::vector<std::pair<uint32_t, uint32_t>>& colliding_pairs,
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
                        // if (collision_test_counter != nullptr)
                            // (*collision_test_counter)++;
                    
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


// stores LTRB, more mem, less math
template<typename T, typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
struct Quadrant
{
    typedef std::array<Quadrant, 4> Subdivision;
    
    T l, t, r, b;

    Quadrant() = default;

    Quadrant(const T l_, const T t_, const T r_, const T b_) : l(l_), t(t_), r(r_), b(b_) {}
    
    bool intersects(const Quadrant & other) const
    {
        return other.l < r && other.t < b && other.r > l && other.b > t;
    }

    bool contains(const T x, const T y) const 
    {
        return l <= x && x <= r && t <= y && y <= b;
    } 

    
# if __cplusplus >= 20170L
    Subdivision divide() const 
    {
        if constexpr (std::is_floating_point_v<T>)
        {
            T mx = (l + r) / 2;
            T my = (t + b) / 2;
            
            return
            {
                Quadrant{l, t, mx, my},
                Quadrant{mx, t, r, my},
                Quadrant{mx, my, r, b},
                Quadrant{l, my, mx, b}
            };
        }
        else
        {
            T mx = (l + r) >> 1;
            T my = (t + b) >> 1;
            
            return  
            {
                Quadrant{l, t, mx, my},
                Quadrant{mx + 1, t, r, my},
                Quadrant{mx + 1, my + 1, r, b},
                Quadrant{l, my, mx + 1, b}
            };
        }
    }
#else
    template<class Q = T>
    typename std::enable_if<std::is_floating_point<Q>::value, Subdivision>::type
    divide() const 
    {
        Q mx = (l + r) / 2;
        Q my = (t + b) / 2;
    
        return Subdivision
        {
            Quadrant{l, t, mx, my},
            Quadrant{mx, t, r, my},
            Quadrant{mx, my, r, b},
            Quadrant{l, my, mx, b}
        };
    }
    
    template<class Q = T>
    typename std::enable_if<std::is_integral<Q>::value, Subdivision>::type
    divide() const 
    {
        Q mx = (l + r) / 2;
        Q my = (t + b) / 2;
    
        return Subdivision
        {
            Quadrant{l, t, mx, my},
            Quadrant{mx + 1, t, r, my},
            Quadrant{mx + 1, my + 1, r, b},
            Quadrant{l, my, mx + 1, b}
        };
    }

#endif
};

// stores center and halve the width of a square, less mem, more math 
template<typename T, typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
struct CenteredQuadrant
{
    typedef std::array<CenteredQuadrant, 4> Subdivision;
    
    T cx, cy, w2;

    CenteredQuadrant() = default;

    CenteredQuadrant(const T cx_, const T cy_, const T w_)
    : cx(cx_), cy(cy_), w2(w_)
    {
        
    }
    
    CenteredQuadrant(const T l_, const T t_, const T r_, const T b_)
    :
        cx((l_ + r_) / 2),
        cy((t_ + b_) / 2),
        w2(std::max(r_ - l_, b_ - t_) / 2)
    {
        
    }
    
    bool intersects(const CenteredQuadrant & other) const
    {
        return std::abs(cx - other.cx) <= w2 + other.w2 && std::abs(cy - other.cy) <= w2 + other.w2;
    }

    bool contains(const T x, const T y) const 
    {
        return std::abs(cx - x) <= w2 && std::abs(cy - y) <= w2;
    } 
    
    Subdivision divide() const 
    {
        T w4 = w2 / 2;
        T cxl = cx - w4;
        T cxr = cx + w4;
        T cyt = cy - w4;
        T cyb = cy + w4;
    
        return Subdivision
        {
            CenteredQuadrant{cxl, cyt, w4},
            CenteredQuadrant{cxr, cyt, w4},
            CenteredQuadrant{cxr, cyb, w4},
            CenteredQuadrant{cxl, cyb, w4},
        };
    }
};

class LazyQuadTreeNode
{
public:
    typedef Quadrant<float> Quad;
    
    explicit LazyQuadTreeNode(const Quad & q_);
    LazyQuadTreeNode* get_next_node(sf::Vector2f loc);

    void push(const Physical* item);

    size_t count_nodes() const ;
    size_t count_items() const ;
    void query_range(const sf::Vector2f & loc, float R, std::vector<const Physical*>& collection) const;
    void query_range(const Quad & loc, std::vector<const Physical*>& collection) const;
    
private:

    Quad quadrant;
    Quad::Subdivision subquadrants;
    std::array<std::unique_ptr<LazyQuadTreeNode>, 4> leaves;
    const Physical* content = nullptr;
    bool has_leaves = false;
};


class LightQuadTreeNode;


class QuadTree : public CollisionDetector
{
public:
    explicit QuadTree(const std::vector<std::shared_ptr<Physical>> & objects) ;
    QuadTree(const QuadTree& _) = delete;
    QuadTree(QuadTree &&_) = delete;
    QuadTree& operator =(QuadTree && _) = delete;
    QuadTree operator =(const QuadTree & _) = delete;

    ~QuadTree() override = default;
    
    void detect_collisions(
        const Physical* that,
        std::vector<std::pair<uint32_t, uint32_t>>& colliding_pairs,
        int* collision_test_counter = nullptr
    ) override;
    
private:
    
    LazyQuadTreeNode quad_tree_root;
    const float max_span;
};


// quadrant is not initialized intentionally, to create node before knowing it 
class LightQuadTreeNode
{
public:
    /* int speedup is not too great but creates a risk of content collisions when going too deep,
     * requiring the ability to store multiple content elements inside a single node */
    
    // typedef Quadrant<int> Quad;
    typedef Quadrant<float> Quad; 
    // typedef CenteredQuadrant<float> Quad; // appears to be slower 
    
    LightQuadTreeNode() = default;
    explicit LightQuadTreeNode(const Quad & q_) : quadrant(q_) {}
    explicit LightQuadTreeNode(Quad & q_) : quadrant(q_) {}
    
    LightQuadTreeNode(const LightQuadTreeNode& _) = default;
    LightQuadTreeNode(LightQuadTreeNode &&_) = default;
    LightQuadTreeNode& operator =(LightQuadTreeNode && _) = default;
    
    LightQuadTreeNode operator =(const LightQuadTreeNode & _) = delete;
    
private:

    Quad quadrant;
    
    int first_leaf = -1; // we dont need to store all the leafs, they are 

    const Physical* content = nullptr;

    friend class LightQuadTree;
};


class LightQuadTree : public CollisionDetector
{
public:

    using Quad = LightQuadTreeNode::Quad;
    
    explicit  LightQuadTree(const std::vector<std::shared_ptr<Physical>> & objects) ;
    
    LightQuadTree(const LightQuadTree& _) = delete;
    LightQuadTree(LightQuadTree &&_) = delete;
    LightQuadTree& operator =(LightQuadTree && _) = delete;
    LightQuadTree operator =(const LightQuadTree & _) = delete;

    ~LightQuadTree() override = default;
    
    void detect_collisions(
        const Physical* that,
        std::vector<std::pair<uint32_t, uint32_t>>& colliding_pairs,
        int* collision_test_counter = nullptr
    ) override;
    
private:

    std::array<LightQuadTreeNode, static_cast<size_t>(MAX_BALLS * 4)> nodes;
    int next_placed_node = 0;
    const float max_span;
        
    int create_new_node(const Quad& q);
    int get_next_node(int pos, sf::Vector2f loc);
    void push(int pos, const Physical* item);
    void query_range(int pos, const sf::Vector2f & loc, float R, std::vector<const Physical*>& collection) const;
    void query_range(int pos, const Quad & loc, std::vector<const Physical*>& collection) const;
};


