#include <SDL3/SDL.h>

/*
 Integrare SDL
 Draw rectangle
 Schelete game engine
 Spawn system
 Spawn 50k+ entities
 Coliziune (patrat, sfera)
 Modificator viteza (din butoane)
 Control nr entitati (un slider)
 Zoom
 ^ OOP STYLE

 Youtube: Javidx9
 */

class Game {
public:
    bool init();
    void run_loop();
private:
    void process_input();
    void update();
    void process_output();

    SDL_Window* window = NULL;
    SDL_Renderer* renderer = NULL;
    bool m_isRunning = false;

    SDL_FRect dummy_rect = { 100, 100, 200, 100 };
};