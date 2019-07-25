#ifndef GAME_H
#define GAME_H

#include <cmath>
#include <iostream>
#include <SDL.h>

#define WIN_WIDTH  480
#define WIN_HEIGHT 320

#define PADDLE_WIDTH        80
#define PADDLE_HEIGHT       10
#define PADDLE_BOTTOM_PAD   10

#define BRICKS_ROWS     4
#define BRICKS_COLUMNS  5
#define NUM_BRICKS      BRICKS_ROWS * BRICKS_COLUMNS
#define BRICKS_LEFTPAD  4
#define BRICKS_TOPPAD   15
#define BRICKS_PAD      4
#define BRICKS_HEIGHT   15

#define DELAY_MS 2000

struct Ball
{
    static const int MAX_SPEED = 6;

    float X, Y;
    float W, H;
    float Vx, Vy;
};

struct BrickParams
{
    static const int LEFT_PAD = 4;
    static const int TOP_PAD = 15;
    static const int INNER_PAD = 4;
    static const int HEIGHT = 15;

    int Width;
    int Height;
};

struct GameState
{
    bool IsRunning;
    bool IsGameOver;
    int NumLives;

    Ball TheBall;
    SDL_Rect ThePaddle;
    SDL_Rect TheBricks[NUM_BRICKS];
    bool IsBrickActive[NUM_BRICKS];

    bool IsDelaying;
    Uint32 LastDelayTick;
};

void InitializeGameState(GameState &State);
void Update(GameState &State, float MouseX);

#endif
