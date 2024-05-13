#pragma once
#include <array>
#include <type_traits>





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
        return other.l <= r && other.t <= b && other.r >= l && other.b >= t;
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
        Q mx = (l + r) >> 1;
        Q my = (t + b) >> 1;
    
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