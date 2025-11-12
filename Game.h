#include <vector>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include "FloatingObject.h"
#include <cmath>
#include "utils.h"
#include "imgui.h"
#include "backends/imgui_impl_sdl3.h"
#include "backends/imgui_impl_sdlrenderer3.h"


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

/*
Faza 1 – Baza proiectului si test de implementare
Sa se dezvolte o aplicatie cu urmatoarele caracteristici:
1. sa fie incorporata in proiectul vostru libraria “SDL” (DONE)
(optional “dead IMGUI” pentru UI) (DONE)
2. sa se poata afisa pe ecran sprite-uri. (DONE)
3. sa se poata selecta numarul de obiecte reprezentate pe ecran cu sprite-uri (pana la un maxim de 100 000 de obiecte) (DONE)
4. sa se implementeze coliziune intre aceste obiecte (DONE)
5. sa se implementeze un mod de a masura performanta programului (FPS-uri, memorie folosita etc.) (DONE)
Punctele 3 si 4 trebuie implementate in doua moduri, Object Oriented si Data Oriented (DOAR PENTRU FAZA 1), de preferat sa putem sa schimbam in timp real intre cele doua optiuni.  (DONE)
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
    void process_imgui_output();

    void check_collisions();
    void dod_check_collisions();

    void add_objects(int quantity);
    void remove_objects(int quantity);

    bool circles_overlap(int id1, int id2);
    bool circles_overlap(FloatingObject* a_fo, FloatingObject* b_fo);

    SDL_Window* window = NULL;
    SDL_Renderer* renderer = NULL;
    SDL_Texture* texture = NULL;
    bool m_isRunning = false;
    float m_fps = 0;
    float m_lastDelta = 0.0f;
    int m_spawn_quantity = 10;
    int m_use_dod = 0;

    std::vector<FloatingObject*> m_objects;
    SDL_FRect dummy_rect = { 0, 0, 200, 100 };




    // Floating objects
    std::vector<int> m_floatingObject_id;
    std::vector<SDL_FPoint> m_floatingObject_position;
    std::vector<SDL_FPoint> m_floatingObject_velocity;
    std::vector<float> m_floatingObject_direction;
    float m_floatingObject_radius = Utils::g_BALL_DIAMETER / 2.0f;

    void m_floatingObject_update(float deltaSpeed, int i);
    void m_floatingObject_render(int i);
    //////////////////////
};