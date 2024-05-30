#ifndef DEFS_H_INCLUDED
#define DEFS_H_INCLUDED

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 608;
const int TILE_MULTIPLIER = 32;
const char* WINDOW_TITLE = "Baitaplon";
const int SPEED = 8;
const int ACCEL = 4;
const int FALL_ACCEL = 1;
const int INITIAL_JUMP_HEIGHT = -14;
const int CUBE_SIZE = 32;
const int PLATFORM_SIZE = 32;
const int LEVEL_COUNT = 2;
const int SPIKE_LEFT_HITBOX = 10;
const int SPIKE_RIGHT_HITBOX = 22;
const int SPIKE_TOP_HITBOX = 8;
const int SPIKE_BOTTOM_HITBOX = 24;


enum horizontal{
    LEFT_MOVE = -1,
    NO_H_MOVE = 0,
    RIGHT_MOVE = 1,
};


#endif // DEFS_H_INCLUDED
