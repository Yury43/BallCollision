#pragma once

#include <SFML/System/Vector2.hpp>
#include <vector>
#include "vector_math.h"

// Abstract physical object parenting Ball and (previously) Line 

class Collidable
{
public:
    const uint32_t id;
    sf::Vector2f p = { 0, 0 };
    
    Collidable() : id(next_id++) { }

    Collidable(const Collidable &_) = delete;
    Collidable(Collidable &&_) = delete;
    
    virtual ~Collidable() = default;

    virtual bool is_touching(const Collidable* other) const = 0;
    virtual float span() const = 0;
    
    virtual void handle_collision(Collidable* other) = 0;
    virtual void update_position(float deltaTime) = 0;
    virtual void apply_reactions() = 0;
    virtual bool handle_wall_collision(float left, float top, float right, float bottom) = 0;
    virtual std::unique_ptr<sf::Shape> get_drawing_shape() = 0;
    
    virtual void mark_colliding(bool is_colliding) { }
    
protected:

    // The result of collision applied on object


    
private:
    static uint32_t next_id;
};

 


