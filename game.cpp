
#include "game.h"

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

// TODO Fix this dynamic sizing of brick
static BrickParams Calculate_BrickParams(int Width, int LPad, int NumCols)
{
    BrickParams Params;
    Params.Height = BRICKS_HEIGHT;

    int Available_Brick_Width = Width - 2*LPad - (NumCols * BRICKS_PAD) + BRICKS_PAD;
    Params.Width = Available_Brick_Width / NumCols;

    return Params;
}

void InitializeGameState(GameState &State)
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
    return 2.0f * Ball::MAX_SPEED / Paddle.w * Distance;
}

// NOTE GameState is a candidate to be an object, but I only have one so...
void Update(GameState &State, float MouseX)
{
    if (State.NumLives == 0)
    {
        State.IsGameOver = true;
        std::cout << "Game Over!" << std::endl;
        return;
    }

    // TODO make paddle move with velocity and dt
    State.ThePaddle.x = MouseX - PADDLE_WIDTH / 2;

    if (State.IsDelaying)
    {
        if ((SDL_GetTicks() - State.LastDelayTick) >= DELAY_MS)
        {
            State.IsDelaying = false;
        }
        else
        {
            return; // do nothing if delaying
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
        State.TheBall.Vy = -sqrt(Ball::MAX_SPEED * Ball::MAX_SPEED - State.TheBall.Vx * State.TheBall.Vx);
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
                if ((BallLeft < BrickRight) && (BallRight > BrickRight))
                {
                    State.TheBall.Vx = -State.TheBall.Vx;
                }
                // Left of brick
                else if ((BallRight > BrickLeft) && (BallLeft < BrickLeft))
                {
                    State.TheBall.Vx = -State.TheBall.Vx;
                }
                else
                {
                    State.TheBall.Vy = -State.TheBall.Vy;
                }

                if (NoBricksRemaining(State.IsBrickActive))
                {
                    State.IsGameOver = true;
                    std::cout << "You Win!" << std::endl;
                    return;
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
