#pragma once
#include "physicals.h"

struct Reaction
{
    sf::Vector2f velocity = { 0, 0 };
    sf::Vector2f corrected_position = { 0, 0 };
};

class Ball : public Collidable
{
public:
    sf::Vector2f dir = { 0, 0 };
    float R = 0;
    float speed = 0;
    sf::Color color = sf::Color::White;

    Ball() = default;

    Ball(const Ball& other);
    Ball(Ball &&_) = delete;
    Ball operator =(const Ball & _) = delete;
    Ball& operator =(const Ball && _) = delete;
    
    ~Ball() override = default;

    bool is_touching(const Collidable* other) const override;

    void mark_colliding(bool is_colliding) override;
    void handle_collision(Collidable* other) override;
    void update_position(float deltaTime) override;
    std::unique_ptr<sf::Shape> get_drawing_shape() override;

    float mass() const;
    sf::Vector2f velocity() const;
    sf::Vector2f momentum() const;
    double energy() const;

    bool handle_wall_collision(float left, float top, float right, float bottom) override;
    void apply_reactions() override;
    float span() const override {return R;}
    
private:
    std::vector<Reaction> reactions = {};
};


