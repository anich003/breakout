#include "game.h"
#include "render.h"

#define GAME_HZ 60

int main()
{
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window *Window = SDL_CreateWindow("Breakout",
                                          SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          WIN_WIDTH, WIN_HEIGHT,
                                          SDL_WINDOW_SHOWN);

    SDL_Renderer *Renderer = SDL_CreateRenderer(Window, -1, SDL_RENDERER_ACCELERATED);

    GameState TheGameState;
    InitializeGameState(TheGameState);

    int MouseX, MouseY;

    float MSPerFrame = 1.0f / GAME_HZ * 1000.0f;
    float ElapsedMS;
    Uint32 LastTick = SDL_GetTicks();

    while (TheGameState.IsRunning)
    {
        ElapsedMS = (SDL_GetTicks() - LastTick);

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
        SDL_GetMouseState(&MouseX, &MouseY);

        if (ElapsedMS >= MSPerFrame)
        {
            Update(TheGameState, MouseX);

            Render(Renderer, TheGameState);

            LastTick = SDL_GetTicks();
        }
    }

    SDL_DestroyWindow(Window);
    SDL_Quit();

    return 0;
}
