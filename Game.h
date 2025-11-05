#include <vector>
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
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
    void update(float deltaTime);
    void process_output();

    void check_collisions();
    void add_object();
    void print_text(const char* text, SDL_FPoint coords);

    SDL_Window* window = NULL;
    SDL_Renderer* renderer = NULL;
    SDL_Texture* texture = NULL;
    TTF_Font* roboto_font = NULL;
    bool m_isRunning = false;
    float m_fps = 0;
    float m_lastDelta = 0.0f;

    std::vector<FloatingObject*> m_objects;
    SDL_FRect dummy_rect = { 0, 0, 200, 100 };
};