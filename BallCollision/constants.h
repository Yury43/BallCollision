#pragma once

constexpr int WINDOW_X = 1024;
constexpr int WINDOW_Y = 768;
//constexpr int WINDOW_Y = 1024;
constexpr int MAX_BALLS = 300;
constexpr int MIN_BALLS = 100;
constexpr float M_PI = 3.1415926;
constexpr int TARGET_FRAMERATE = 60;
//constexpr bool ASYNC_PHYS = false; // run simulation is the same thread as the window 
constexpr bool ASYNC_PHYS = true; // run simulation in a thread separate from the window 
//constexpr bool MOVE_BEFORE_COLLISION = false; // Update positions after searching for collisions to render state preceding collision of next iteration 
constexpr bool MOVE_BEFORE_COLLISION = true; // Update positions before searching for collisions to account for current positions results in a more accurate sim 