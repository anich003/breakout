#include <SDL.h>
#include <cmath>
#include <iostream>

#define WIN_WIDTH  480
#define WIN_HEIGHT 320
#define GAME_HZ 60

#define PADDLE_WIDTH        80
#define PADDLE_HEIGHT       10
#define PADDLE_BOTTOM_PAD   10

#define BALL_SPEED      5
#define BALL_SPEED_XMAX 4

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

static SDL_Rect Make_Paddle()
{
    SDL_Rect Paddle;
    Paddle.x = 70;
    Paddle.y = WIN_HEIGHT - PADDLE_BOTTOM_PAD - PADDLE_HEIGHT;
    Paddle.w = PADDLE_WIDTH;
    Paddle.h = PADDLE_HEIGHT;
    return Paddle;
}

static bool Collide_AABB(int L1, int R1, int T1, int B1, int L2, int R2, int T2, int B2)
{
    return ((L2 < R1) && (L1 < R2)) && ((T2 < B1) && (T1 < B2));
}

static bool Collide_AABB(SDL_Rect A, SDL_Rect B)
{
    return Collide_AABB(A.x, A.x + A.w, A.y, A.y + A.h, B.x, B.x + B.w, B.y, B.y + B.h);
}

struct BrickParams
{
    int Width, Height;
};

struct GameState
{
    bool IsRunning;
    bool IsGameOver;
    int NumLives;

    bool IsDelaying;
    Uint32 LastDelayTick;
};

static BrickParams Calculate_BrickParams(int Width, int LPad, int NumCols)
{
    BrickParams Params;
    Params.Height = BRICKS_HEIGHT;

    int Available_Brick_Width = Width - 2*LPad - (NumCols * BRICKS_PAD) + BRICKS_PAD;
    Params.Width = Available_Brick_Width / NumCols;

    return Params;
}

static float Calculate_Bounce(const Ball &B, const SDL_Rect &Paddle)
{
    float Distance = (B.X + B.W/2) - (Paddle.x + Paddle.w/2);
    return 2.0f * BALL_SPEED_XMAX / Paddle.w * Distance;
}

static bool NoBricksRemaining(bool *IsBrickActive)
{
    for(int i=0; i<NUM_BRICKS; i++)
        if (IsBrickActive[i]) return false;
    return true;
}

static void Update(Ball &B, SDL_Rect &Paddle, float MouseX, SDL_Rect *TheBricks, bool *IsBrickActive, GameState &State)
{
    if (State.NumLives == 0)
    {
        State.IsGameOver = true;
        return;
    }

    if (NoBricksRemaining(IsBrickActive))
    {
        State.IsGameOver = true;
        std::cout << "You Win!" << std::endl;
        return;
    }

    Paddle.x = MouseX - PADDLE_WIDTH / 2;

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
    SDL_Rect BallRect = { B.X, B.Y, B.W, B.H };
    if (Collide_AABB(BallRect, Paddle))
    {
        B.Vx = Calculate_Bounce(B, Paddle);
        B.Vy = -sqrt(BALL_SPEED * BALL_SPEED - B.Vx * B.Vx);
    }

    // Collision with bricks
    for(int i=0; i<NUM_BRICKS; i++)
    {
        if (IsBrickActive[i])
        {
            if (Collide_AABB(BallRect, TheBricks[i]))
            {
                IsBrickActive[i] = false;

                int BrickRight = TheBricks[i].x + TheBricks[i].w;
                int BrickLeft = TheBricks[i].x;
                int BallLeft = B.X;
                int BallRight = B.X + B.W;
                // Right of brick
                if ((BallLeft <= BrickRight) && (BallRight > BrickRight))
                {
                    B.Vx = -B.Vx;
                }
                // Left of brick
                else if ((BallRight >= BrickLeft) && (BallLeft < BrickLeft))
                {
                    B.Vx = -B.Vx;
                }
                else
                {
                    B.Vy = -B.Vy;
                }
            }
        }
    }

    // Collision with borders
    if (B.X < 0)                 B.Vx = -B.Vx;
    if (B.Y < 0 + BRICKS_TOPPAD) B.Vy = -B.Vy;
    if (B.X + B.W >= WIN_WIDTH)  B.Vx = -B.Vx;

    // Lose life and reset/game over
    if (B.Y + B.H >= WIN_HEIGHT)
    {
        State.NumLives -= 1;
        State.IsDelaying = true;
        State.LastDelayTick = SDL_GetTicks();
        B.X = WIN_WIDTH / 2;
        B.Y = WIN_HEIGHT / 2 - 15;
        B.Vx = 3;
        B.Vy = 4;
    }

    B.X += B.Vx;
    B.Y += B.Vy;
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

static void Render(SDL_Renderer *Renderer, const Ball &B, const SDL_Rect &Paddle, 
                   SDL_Rect *TheBricks, bool *IsBrickActive, const GameState &State)
{
    SDL_SetRenderDrawColor(Renderer, 255, 255, 255, 0);
    SDL_Rect BallRect = {B.X, B.Y, B.W, B.H};
    SDL_RenderFillRect(Renderer, &BallRect);
    SDL_RenderFillRect(Renderer, &Paddle);

    for(int i=0; i<NUM_BRICKS; i++)
    {
        if (IsBrickActive[i])
        {
            SDL_RenderFillRect(Renderer, &TheBricks[i]);
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

    int MouseX, MouseY;

    unsigned int MSPerFrame = 1.0f / GAME_HZ * 1000.0f;
    Uint32 LastTick = SDL_GetTicks();

    GameState TheGameState;
    TheGameState.IsRunning = true;
    TheGameState.IsGameOver = false;
    TheGameState.NumLives = 3;
    TheGameState.IsDelaying = false;

    Ball TheBall;
    TheBall.X = WIN_WIDTH / 2;
    TheBall.Y = WIN_HEIGHT / 2;
    TheBall.W = 10;
    TheBall.H = 10;
    TheBall.Vx = 3;
    TheBall.Vy = 3;

    SDL_Rect ThePaddle = Make_Paddle();
    BrickParams Params = Calculate_BrickParams(WIN_WIDTH, BRICKS_LEFTPAD, BRICKS_COLUMNS);
    SDL_Rect TheBricks[NUM_BRICKS];
    bool IsBrickActive[NUM_BRICKS]; 
    for(int i=0; i<NUM_BRICKS; i++)
    {
        IsBrickActive[i] = true;

        int row = i / BRICKS_COLUMNS;
        int col = i % BRICKS_COLUMNS;
        TheBricks[i].x = BRICKS_LEFTPAD + col * (Params.Width + BRICKS_PAD);
        TheBricks[i].y = BRICKS_TOPPAD + row * (Params.Height + BRICKS_PAD);
        TheBricks[i].w = Params.Width;
        TheBricks[i].h = Params.Height;
    }

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
                case SDL_MOUSEMOTION:
                {
                    SDL_GetMouseState(&MouseX, &MouseY);    
                }
            }
        }

        if (TheGameState.IsGameOver)
        {
            TheGameState.IsRunning = false;
            break;
        }

        Update(TheBall, ThePaddle, MouseX, TheBricks, IsBrickActive, TheGameState);

        SDL_SetRenderDrawColor(Renderer, 0, 0, 0, 0);
        SDL_RenderClear(Renderer);
        Render(Renderer, TheBall, ThePaddle, TheBricks, IsBrickActive, TheGameState);
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
