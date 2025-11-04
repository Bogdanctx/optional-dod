#include <vector>
#include <SDL3/SDL.h>
#include "FloatingObject.h"
#include <cmath>

/*
 Integrare SDL (Done)
 Draw rectangle (Done)
 Schelete game engine (Done)
 Spawn system (Done)
 Spawn 50k+ entities (Done)
 Coliziune (patrat, sfera) (Done)
 Modificator viteza (din butoane) (Done)
 Control nr entitati (un slider)
 Zoom
 ^ OOP STYLE (crapa la 50k unitati)

 Youtube: Javidx9
 */

class Game {
public:
    ~Game();

    bool init();
    void run_loop();
private:
    void process_input();
    void update();
    void process_output();

    void check_collisions();
    void add_object();

    SDL_Window* window = NULL;
    SDL_Renderer* renderer = NULL;
    SDL_Texture* texture = NULL;
    bool m_isRunning = false;

    std::vector<FloatingObject*> m_objects;
    SDL_FRect dummy_rect = { 100, 100, 200, 100 };
};