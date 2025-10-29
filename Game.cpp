#include "Game.h"
#include <SDL3/SDL_keycode.h>

#include <cstdio>

bool Game::init() {
    SDL_Init(SDL_INIT_VIDEO);
    window = SDL_CreateWindow("Proiect DOD", 1200, 800, SDL_WINDOW_OPENGL);

    if (window == NULL) {
        return false;
    }

    renderer = SDL_CreateRenderer(window, NULL);

    if (renderer == NULL) {
        return false;
    }

    m_isRunning = true;

    return true;
}


void Game::run_loop() {
    while (m_isRunning) {
        process_input();
        update();
        process_output();
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void Game::process_input() {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_EVENT_QUIT: {
                m_isRunning = false;
                break;
            }
            case SDL_EVENT_KEY_DOWN: {
                SDL_KeyboardEvent keyboard_event = event.key;
                SDL_Keycode key_pressed = keyboard_event.key;

                if (key_pressed == SDLK_ESCAPE) {
                    m_isRunning = false;
                }

                break;
            }
            default:
                break;
        }
    }
}

void Game::update() {

}

void Game::process_output() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 0, 0, 255, SDL_ALPHA_OPAQUE);
    SDL_RenderFillRect(renderer, &dummy_rect);

    SDL_RenderPresent(renderer);
}