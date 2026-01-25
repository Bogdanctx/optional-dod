#pragma once
#include <vector>
#include <memory>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include "Actor.h"
#include "SpatialGrid.h"
#include "constants.h"
#include "imgui.h"
#include "backends/imgui_impl_sdl3.h"
#include "backends/imgui_impl_sdlrenderer3.h"
#include "Chronometer.h"
#include "MathUtils.h"
#include "actor_type.h"

class Game {
public:
    Game();
    ~Game();

    bool init();
    void run_loop();

private:
    void process_input();
    void update(float deltaTime);
    void render();
    
    void update_imgui();
    void apply_physics(float deltaTime);
    void optimized_resolve_collisions(); // coliziuni cu grid
    void naive_resolve_collisions(); // coliziuni O(n^2)
    void enforce_boundaries(float& x, float& y, float& vx, float& vy); // pt a preveni iesirea din ecran
    void manage_entity_count(); // functie care manageriaza nr de obiecte de pe ecran
    void check_screen_bounds();
    void update_health_status(int i, int j); // actualizeaza status-ul fiecarui actor (healthy, healed, sick, etc.)

    // sincronizare intre OOP si DOD
    void sync_state_to_dod();
    void sync_state_to_oop();

    const int NUMBER_OF_STATES = 6; // cate states am in status.h

    SDL_Window* window = NULL;
    SDL_Renderer* renderer = NULL;
    std::vector<SDL_Texture*> state_textures; // retine textura corespunzatoare fiecarui state
    
    bool m_isRunning = false;
    bool m_use_dod = false;
    bool m_apply_collisions = true;
    bool m_optimizedCollisions = true;
    bool m_drawGrid = false;

    float m_virusSeverity = 50.0f;

    // date pt a masura performanta
    float m_fps = 0;
    float m_processInputTime = 0;
    float m_updateTime = 0;
    float m_renderTime = 0;
    Uint64 m_perfStart = 0;

    long long oop_bytes = 0;
    long long dod_bytes = 0;
    long long grid_bytes = 0;
    void calculate_memory();

    
    int m_spawn_quantity = 0;
    const int MAX_ENTITIES = 100000;
    int m_lastUsedId = 0;

    std::vector<Actor*> m_actors; // retine obiectele din modul OOP

    // vectori pt modul DOD
    std::vector<int> m_actor_ids;
    std::vector<float> m_actor_positions_x;
    std::vector<float> m_actor_positions_y;
    std::vector<float> m_actor_velocities_x;
    std::vector<float> m_actors_velocities_y;
    std::vector<int> m_actor_types;


    int m_gridCellSize = 50;
    SpatialGrid m_grid;
};