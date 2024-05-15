#pragma once

#include "Colliders/Collidable.hpp"
#include "vector_math.h"

// Abstract physical object parenting Ball and (previously) Line 


class Physical : public Collidable
{
public:
    
    Physical() : Collidable(next_id++) { }

    Physical(const Physical &_) = delete;
    Physical(Physical &&_) = delete;
    
    ~Physical() override = default;

    virtual bool is_touching(const Collidable* other) const = 0;
    
    
    virtual void handle_collision(Physical* other) = 0;
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

 


