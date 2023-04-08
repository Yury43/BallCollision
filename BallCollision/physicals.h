#pragma once

#include <SFML/System/Vector2.hpp>
#include <vector>
#include "vector_math.h"
#include <iostream>

// Abstract physical object parenting Ball and (previously) Line 

class Physical
{
public:
    const uint32_t id;
    sf::Vector2f p = { 0, 0 };
    
    Physical() : id(next_id++) 
    {
        // std::cout << "created Physical #" << id << std::endl; 
    }

    Physical(const Physical &_) = delete;
    Physical(Physical &&_) = delete;
    
    virtual ~Physical() = default;

    virtual bool is_touching(const Physical* other) const = 0;
    virtual void mark_colliding(bool is_colliding) {}
    virtual void handle_collision(Physical* other) = 0;
    virtual void update_position(float deltaTime) = 0;
    virtual std::shared_ptr<sf::Shape> get_drawing_shape() = 0;
    virtual void apply_reactions() = 0;
    virtual bool test_and_handle_wall_collision(float left, float top, float right, float bottom) = 0;
    virtual float span() const = 0;
protected:
    
    
private:
    static uint32_t next_id;
};

// The result of collision applied on object 
struct Reaction
{
    sf::Vector2f velocity = { 0, 0 };
    sf::Vector2f corrected_position = { 0, 0 };
};


