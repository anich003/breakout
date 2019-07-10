#include <SDL.h>
#include <cmath>
#include <iostream>

#define WIN_WIDTH  480
#define WIN_HEIGHT 320
#define GAME_HZ 60

#define PADDLE_WIDTH        80
#define PADDLE_HEIGHT       10
#define PADDLE_BOTTOM_PAD   10

#define BALL_SPEED      6
#define BALL_SPEED_XMAX 5

#define BRICKS_ROWS     5
#define BRICKS_COLUMNS  7
#define NUM_BRICKS      BRICKS_ROWS * BRICKS_COLUMNS
#define BRICKS_LEFTPAD  4
#define BRICKS_TOPPAD   15
#define BRICKS_PAD      4
#define BRICKS_HEIGHT   15

#define DELAY_MS 2000

struct Ball
{
    float X, Y;
    float W, H;
    float Vx, Vy;
};

struct BrickParams
{
    int Width, Height;
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

static Ball Make_Ball()
{
    Ball ABall;
    ABall.X = WIN_WIDTH / 2;
    ABall.Y = WIN_HEIGHT / 2;
    ABall.W = 10;
    ABall.H = 10;
    ABall.Vx = 3;
    ABall.Vy = 3;

    return ABall;
}

static SDL_Rect Make_Paddle()
{
    SDL_Rect Paddle;
    Paddle.x = 70;
    Paddle.y = WIN_HEIGHT - PADDLE_BOTTOM_PAD - PADDLE_HEIGHT;
    Paddle.w = PADDLE_WIDTH;
    Paddle.h = PADDLE_HEIGHT;
    return Paddle;
}

static BrickParams Calculate_BrickParams(int Width, int LPad, int NumCols)
{
    BrickParams Params;
    Params.Height = BRICKS_HEIGHT;

    int Available_Brick_Width = Width - 2*LPad - (NumCols * BRICKS_PAD) + BRICKS_PAD;
    Params.Width = Available_Brick_Width / NumCols;

    return Params;
}

static void InitializeGameState(GameState &State)
{
    State.IsRunning = true;
    State.IsGameOver = false;
    State.NumLives = 3;
    State.IsDelaying = false;

    State.TheBall = Make_Ball();
    State.ThePaddle = Make_Paddle();

    BrickParams Params = Calculate_BrickParams(WIN_WIDTH, BRICKS_LEFTPAD, BRICKS_COLUMNS);
    for(int i=0; i<NUM_BRICKS; i++)
    {
        State.IsBrickActive[i] = true;

        int row = i / BRICKS_COLUMNS;
        int col = i % BRICKS_COLUMNS;
        State.TheBricks[i].x = BRICKS_LEFTPAD + col * (Params.Width + BRICKS_PAD);
        State.TheBricks[i].y = BRICKS_TOPPAD + row * (Params.Height + BRICKS_PAD);
        State.TheBricks[i].w = Params.Width;
        State.TheBricks[i].h = Params.Height;
    }
}

static bool NoBricksRemaining(bool *IsBrickActive)
{
    for(int i=0; i<NUM_BRICKS; i++)
        if (IsBrickActive[i]) return false;
    return true;
}

static bool Collide_AABB(int L1, int R1, int T1, int B1, int L2, int R2, int T2, int B2)
{
    return ((L2 < R1) && (L1 < R2)) && ((T2 < B1) && (T1 < B2));
}

static bool Collide_AABB(SDL_Rect A, SDL_Rect B)
{
    return Collide_AABB(A.x, A.x + A.w, A.y, A.y + A.h, B.x, B.x + B.w, B.y, B.y + B.h);
}

static float Calculate_Bounce(const Ball &B, const SDL_Rect &Paddle)
{
    float Distance = (B.X + B.W/2) - (Paddle.x + Paddle.w/2);
    return 2.0f * BALL_SPEED_XMAX / Paddle.w * Distance;
}

static void Update(GameState &State, float MouseX)
{
    if (State.NumLives == 0)
    {
        State.IsGameOver = true;
        std::cout << "Game Over!" << std::endl;
        return;
    }

    if (NoBricksRemaining(State.IsBrickActive))
    {
        State.IsGameOver = true;
        std::cout << "You Win!" << std::endl;
        return;
    }

    State.ThePaddle.x = MouseX - PADDLE_WIDTH / 2;

    if (State.IsDelaying)
    {
        if ((SDL_GetTicks() - State.LastDelayTick) >= DELAY_MS)
        {
            State.IsDelaying = false;
        }
        else
        {
            return;
        }
    }
    
    // Collision with paddle
    SDL_Rect BallRect = { 
        State.TheBall.X, 
        State.TheBall.Y, 
        State.TheBall.W, 
        State.TheBall.H 
    };
    if (Collide_AABB(BallRect, State.ThePaddle))
    {
        State.TheBall.Vx = Calculate_Bounce(State.TheBall, State.ThePaddle);
        State.TheBall.Vy = -sqrt(BALL_SPEED * BALL_SPEED - State.TheBall.Vx * State.TheBall.Vx);
    }

    // Collision with bricks
    for(int i=0; i<NUM_BRICKS; i++)
    {
        if (State.IsBrickActive[i])
        {
            if (Collide_AABB(BallRect, State.TheBricks[i]))
            {
                State.IsBrickActive[i] = false;

                int BrickRight = State.TheBricks[i].x + State.TheBricks[i].w;
                int BrickLeft = State.TheBricks[i].x;
                int BallLeft = State.TheBall.X;
                int BallRight = State.TheBall.X + State.TheBall.W;
                // Right of brick
                if ((BallLeft <= BrickRight) && (BallRight > BrickRight))
                {
                    State.TheBall.Vx = -State.TheBall.Vx;
                }
                // Left of brick
                else if ((BallRight >= BrickLeft) && (BallLeft < BrickLeft))
                {
                    State.TheBall.Vx = -State.TheBall.Vx;
                }
                else
                {
                    State.TheBall.Vy = -State.TheBall.Vy;
                }
            }
        }
    }

    // Collision with borders
    if (State.TheBall.X < 0)                                State.TheBall.Vx = -State.TheBall.Vx;
    if (State.TheBall.Y < 0 + BRICKS_TOPPAD)                State.TheBall.Vy = -State.TheBall.Vy;
    if (State.TheBall.X + State.TheBall.W >= WIN_WIDTH)     State.TheBall.Vx = -State.TheBall.Vx;

    // Lose life and reset/game over
    if (State.TheBall.Y + State.TheBall.H >= WIN_HEIGHT)
    {
        State.NumLives -= 1;
        State.IsDelaying = true;
        State.LastDelayTick = SDL_GetTicks();
        State.TheBall.X = WIN_WIDTH / 2;
        State.TheBall.Y = WIN_HEIGHT / 2 - 15;
        State.TheBall.Vx = 3;
        State.TheBall.Vy = 4;
    }

    State.TheBall.X += State.TheBall.Vx;
    State.TheBall.Y += State.TheBall.Vy;
}

static void RenderLives(SDL_Renderer *Renderer, int NumLives)
{
    SDL_SetRenderDrawColor(Renderer, 255, 255, 255, 0);
    SDL_Rect LifeRect = { WIN_WIDTH - 30, 3, 8, 8 };
    SDL_RenderFillRect(Renderer, &LifeRect);
    if (NumLives >= 2)
    {
        LifeRect.x -= 8 + 5;
        SDL_RenderFillRect(Renderer, &LifeRect);
    }
    if (NumLives >= 3)
    {
        LifeRect.x -= 8 + 5;
        SDL_RenderFillRect(Renderer, &LifeRect);
    }
}

static void Render(SDL_Renderer *Renderer, const GameState &State)
{
    SDL_SetRenderDrawColor(Renderer, 255, 255, 255, 0);
    SDL_Rect BallRect = {State.TheBall.X, State.TheBall.Y, State.TheBall.W, State.TheBall.H};
    SDL_RenderFillRect(Renderer, &BallRect);
    SDL_RenderFillRect(Renderer, &State.ThePaddle);

    for(int i=0; i<NUM_BRICKS; i++)
    {
        if (State.IsBrickActive[i])
        {
            SDL_RenderFillRect(Renderer, &State.TheBricks[i]);
        }
    }

    RenderLives(Renderer, State.NumLives);
}


int main()
{
    SDL_Window *Window = SDL_CreateWindow("Breakout",
                                          SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          WIN_WIDTH, WIN_HEIGHT,
                                          SDL_WINDOW_SHOWN);

    SDL_Renderer *Renderer = SDL_CreateRenderer(Window, -1, SDL_RENDERER_ACCELERATED);

    GameState TheGameState;
    InitializeGameState(TheGameState);

    int MouseX, MouseY;

    unsigned int MSPerFrame = 1.0f / GAME_HZ * 1000.0f;
    Uint32 LastTick = SDL_GetTicks();

    while (TheGameState.IsRunning)
    {
        SDL_Event Event;
        while (SDL_PollEvent(&Event))
        {
            switch (Event.type)
            {
                case SDL_QUIT:
                {
                    TheGameState.IsRunning = false;
                    break;
                }
            }
        }

        if (TheGameState.IsGameOver)
        {
            TheGameState.IsRunning = false;
            break;
        }

        int Buttons = SDL_GetMouseState(&MouseX, &MouseY);
        Update(TheGameState, MouseX);

        SDL_SetRenderDrawColor(Renderer, 0, 0, 0, 0);
        SDL_RenderClear(Renderer);

        Render(Renderer, TheGameState);

        SDL_RenderPresent(Renderer);

        while ((SDL_GetTicks() - LastTick) <= MSPerFrame)
        {
            // Delay instead of spin locking?
        }
        LastTick = SDL_GetTicks();
    }

    SDL_Quit();
    return 0;
}
