#include <iostream>
#include "SFML/Graphics.hpp"
#include "MiddleAverageFilter.h"
#include "constants.h"
#include "engine_worker.h"


// TODO to prevent fast balls from escaping and for more accurate model in general one should consider positions on the next tick instead of current one


void draw_fps(sf::RenderWindow& window, float fps)
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
    float lastime = clock.restart().asSeconds();
    Math::MiddleAverageFilter<float, 100> fpscounter;

    while (window.isOpen())
    {
        sf::Event event;
        float poll_start = clock.getElapsedTime().asSeconds();
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }
        }
        float poll_end = clock.getElapsedTime().asSeconds();

        float current_time = clock.getElapsedTime().asSeconds();
        float deltaTime = (current_time - lastime);
        float simDeltaTime = deltaTime - (poll_end - poll_start);
        fpscounter.push(1.0f / (current_time - lastime));
        lastime = current_time;


        std::vector<sf::CircleShape> gballs = sim.get_update(simDeltaTime);

        if (!gballs.empty())
        {
            window.clear();
            for (const auto& gball : gballs)
            {
                window.draw(gball);
            }
        }

        draw_fps(window, fpscounter.getAverage());
        window.display();
    }

    return 0;
}
