#pragma once
#include "SFML/Graphics.hpp"
#include <numeric>


inline float dot(const sf::Vector2f& a, const sf::Vector2f& b)
{
    return a.x * b.x + a.y * b.y;
}


//inline float det(const sf::Vector2f& a, const sf::Vector2f& b)
//{
//    return a.x * b.y - a.y * b.x;
//}

inline float norm(const sf::Vector2f& a)
{
    return std::sqrtf(dot(a, a));
}


inline float dist(const sf::Vector2f& a, const sf::Vector2f& b)
{
    return norm(a - b);
}

inline float dist_squared(const sf::Vector2f& a, const sf::Vector2f& b)
{
    return (a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y);
}

inline sf::Vector2f normalized(const sf::Vector2f& a)
{
    return a / std::max(norm(a), 1e-5f);
}

//inline float angle(const sf::Vector2f& a, const sf::Vector2f& b)
//{
//    return std::acos(dot(a, b) / (norm(a) * norm(b)));
//}
//
//inline float angle_clockwise(const sf::Vector2f& a, const sf::Vector2f& b)
//{
//    return std::atan2(det(a, b), dot(a, b));
//}

//template<typename T>
//sf::Vector2<T> average(const std::vector<sf::Vector2<T>>& items)
//{
//    return std::accumulate(items.begin(), items.end(), sf::Vector2<T>(0, 0)) / static_cast<T>(items.size());
//}

template<typename T>
std::ostream& operator<<(std::ostream& os, const sf::Vector2<T>& v)
{
    os << "( " << v.x << " , " << v.y << " ) ";
    return os;
}