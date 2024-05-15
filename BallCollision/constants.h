#pragma once

#define FIXED_RANDOM_SEED (2)

constexpr int WINDOW_X = 1024 * 2;
constexpr int WINDOW_Y = 768 * 2;
//constexpr int WINDOW_Y = 1024;
constexpr int MAX_BALLS = 200;
constexpr int MIN_BALLS = 200;
constexpr int MIN_R = 2;
constexpr int MAX_R = 12;
constexpr float M_PI = 3.1415926f;
constexpr int TARGET_FRAMERATE = 60;

constexpr bool ASYNC_PHYS = false; // run simulation in   the same thread as the window 
// constexpr bool ASYNC_PHYS = true; // run simulation in a thread separate from the window

/* Order of moving, colliding and rendering steps */ 
constexpr bool MOVE_BEFORE_COLLISION = true;  // "move - collide - render" Dont render incorrect states 
// constexpr bool MOVE_BEFORE_COLLISION = false;   // "move - render - collide" Ensure not to miss the moment of contact 

// limit time of running the program, -1 to run indefinitely  
// constexpr int seconds_to_run_limit = -1;
constexpr int seconds_to_run_limit = 10;

// max number of simulation steps per frame in async mode 
// constexpr float MAX_PHYS_SPEEDUP = 1;
constexpr float MAX_PHYS_SPEEDUP = 4;
// constexpr float MAX_PHYS_SPEEDUP = 8;