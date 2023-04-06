#include <iostream>

#include "SFML/Graphics.hpp"
#include "MiddleAverageFilter.h"
#include "constants.h"
#include "engine_worker.h"


// TODO to prevent fast balls from escaping and for more accurate model in general one should consider positions on the next tick instead of current one


void draw_fps(sf::RenderWindow& window, const float fps)
{
    char c[32];
    snprintf(c, 32, "FPS: %f", fps);
    sf::String str(c);
    window.setTitle(str);
}


int main()
{
    sf::RenderWindow window(sf::VideoMode(WINDOW_X, WINDOW_Y), "ball collision demo");
    window.setFramerateLimit(TARGET_FRAMERATE);

    EngineWorker sim(ASYNC_PHYS);
    sim.run();
    
    sf::Clock clock;
    float last_time = clock.restart().asSeconds();
    Math::MiddleAverageFilter<float, 100> fps_counter;

    while (window.isOpen())
    {
        sf::Event event;
        float poll_start = clock.getElapsedTime().asSeconds();
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed ||
                event.key.code == sf::Keyboard::Escape)
            {
                window.close();
            }
        }
        float poll_end = clock.getElapsedTime().asSeconds();

        float current_time = clock.getElapsedTime().asSeconds();
        float deltaTime = (current_time - last_time);
        float simDeltaTime = deltaTime - (poll_end - poll_start);
        fps_counter.push(1.0f / (current_time - last_time));
        last_time = current_time;


        std::vector<sf::CircleShape> shapes = sim.get_update(simDeltaTime);

        if (!shapes.empty())
        {
            window.clear();
            for (const auto& shape : shapes)
            {
                window.draw(shape);
            }
        }

        draw_fps(window, fps_counter.getAverage());
        window.display();
    }

    return 0;
}
