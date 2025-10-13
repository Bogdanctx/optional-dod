#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

int main() {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *window = SDL_CreateWindow("Proiect DOD", 800, 600, SDL_WINDOW_OPENGL);

    if (window == NULL) {
        return 1;
    }

    bool appIsRunning = true;
    while (appIsRunning) {
        SDL_Event event;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                appIsRunning = false;
            }
        }

        /////////////////


        /////////////////
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}