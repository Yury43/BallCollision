#pragma once
#include "physicals.h"

class Ball : public Physical
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
    Ball operator =(const Ball && _) = delete;
    
    ~Ball() override = default;

    bool is_touching(const Physical* other) const override;

    void mark_colliding(bool is_colliding) override;
    void handle_collision(Physical* other) override;
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


//class Line: public Physical
//{
//public:
//    sf::Vector2f p1;
//    sf::Vector2f p2;
//    sf::Vector2f n;
//
//    Line(const sf::Vector2f& _p1, const sf::Vector2f& _p2) 
//        : Physical(), p1(_p1), p2(_p2), n(normalized(p2 - p1))
//    {
//
//    }
//
//    virtual ~Line(){}
//
//    bool is_touching(const Physical* other) const override;
//    void handle_collision(Physical* other) override;
//};

//static sf::Vector2f project(const Line& l, const sf::Vector2f& p)
//{
//    const auto& a = l.p1;
//    const auto& n = l.n;
//    auto d = p - a;
//    auto pr = d - (dot(d, n) * n);
//    return pr + a;
//}

//static sf::Vector2f project(const sf::Vector2f& a, const sf::Vector2f& b)
//{
//    return dot(a, b) / norm(b) * normalized(b);
//}

//static float dist(const Line& l, const sf::Vector2f& p)
//{
//    return norm(project(l, p));
//}





