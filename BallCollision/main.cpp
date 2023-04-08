#include <cassert>
#include <iostream>

#include "SFML/Graphics.hpp"
#include "MiddleAverageFilter.h"
#include "constants.h"
#include "engine_worker.h"
#include "profiler.h"


// TODO to prevent fast balls from escaping and for more accurate model in general one should consider positions on the next tick instead of current one


void draw_fps(sf::RenderWindow& window, const float fps)
{
    char c[32];
    snprintf(c, 32, "FPS: %f", fps);
    sf::String str(c);
    window.setTitle(str);
}

// std::vector<std::shared_ptr<sf::Shape>> get_drawing_shapes(const std::vector<std::shared_ptr<Physical>> & objects)
// {
//     PROFILE();
//     std::vector<std::shared_ptr<sf::Shape>> shapes;
//     shapes.reserve(objects.size());
//     for (const auto & item : objects)
//     {
//         shapes.push_back(item->get_drawing_shape());
//     }
//     return shapes;
// }

int main()
{    
    sf::RenderWindow window(sf::VideoMode(WINDOW_X, WINDOW_Y), "ball collision demo");
    window.setFramerateLimit(TARGET_FRAMERATE);

    EngineWorker sim(ASYNC_PHYS);

    sim.run();
    
    sf::Clock clock;
    float last_time = clock.restart().asSeconds();
    Math::MiddleAverageFilter<float, 100> fps_counter;

    if (!sim.is_running())
    {
        std::cout << "PRESS SPACE TO START" << std::endl;
    }

    auto start = std::chrono::system_clock::now();
    const int seconds_to_run_limit = 10;
    
    while (window.isOpen())
    {
        sf::Event event;
        float poll_start = clock.getElapsedTime().asSeconds();
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed
                // || event.key.code == sf::Keyboard::Escape
                )
            {
                std::cout << "event.key: " << event.key.code << std::endl;
                window.close();
            }
            else if (event.key.code == sf::Keyboard::Space)
            {
                if (!sim.is_running())
                {
                    sim.run();
                }
            }
        }
        float poll_end = clock.getElapsedTime().asSeconds();

        float current_time = clock.getElapsedTime().asSeconds();
        float deltaTime = current_time - last_time;
        float simDeltaTime = deltaTime - (poll_end - poll_start);
        fps_counter.push(1.0f / (current_time - last_time));
        last_time = current_time;


        if (sim.is_running())
        {
            std::vector<std::shared_ptr<Physical>> drawables = sim.get_update(simDeltaTime);

            if (!drawables.empty())
            {
                window.clear();
                for (const auto& drawable : drawables)
                {
                    std::unique_ptr<sf::Shape> pp = drawable->get_drawing_shape();
                    window.draw(*pp);
                }
            }
        }
        

        draw_fps(window, fps_counter.getAverage());
        window.display();

        auto running_duration = std::chrono::system_clock::now() - start;
        double running_for_seconds = std::chrono::duration_cast<std::chrono::seconds>(running_duration).count(); 
        if (running_for_seconds >= seconds_to_run_limit)
        {
            std::cout << "Have been running for " << running_for_seconds << ", shutting down" << std::endl;
            break;
        }
    }

    sim.stop();
    
    ProfilingTimer::print_stats();
    
    return 0;
}
