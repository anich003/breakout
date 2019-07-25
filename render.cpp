#include "render.h"

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

static void RenderGameOver(SDL_Renderer *Renderer)
{
    SDL_SetRenderDrawColor(Renderer, 255, 255, 255, 0);
    SDL_RenderClear(Renderer);
}

static void RenderGame(SDL_Renderer *Renderer, const GameState &State)
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

void Render(SDL_Renderer *Renderer, const GameState &State)
{
    SDL_SetRenderDrawColor(Renderer, 0, 0, 0, 0);
    SDL_RenderClear(Renderer);

    if (State.IsGameOver)
    {
        RenderGameOver(Renderer);
    }
    else
    {
        RenderGame(Renderer, State);
    }

    SDL_RenderPresent(Renderer);
}



