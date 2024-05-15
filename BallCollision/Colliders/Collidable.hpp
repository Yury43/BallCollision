#pragma once
#include <cstdint>
#include <SFML/System/Vector2.hpp>

class Collidable
{
public:
    Collidable(const int id_) : id(id_){}
    virtual ~Collidable() = default;
    const uint32_t id;
    sf::Vector2f p = { 0, 0 };
    virtual float span() const = 0;
};
