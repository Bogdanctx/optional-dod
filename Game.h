#pragma once
#include <vector>
#include <memory>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include "FloatingObject.h"
#include "SpatialGrid.h"
#include "constants.h"
#include "imgui.h"
#include "backends/imgui_impl_sdl3.h"
#include "backends/imgui_impl_sdlrenderer3.h"
#include "Chronometer.h"
#include "MathUtils.h"

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
    void optimized_resolve_collisions();
    void naive_resolve_collisions();
    void enforce_boundaries();
    void manage_entity_count();
    void check_screen_bounds(float& x, float& y, float& vx, float& vy);

    // sincronizare intre OOP si DOD
    void sync_state_to_dod();
    void sync_state_to_oop();

    
    SDL_Window* window = NULL;
    SDL_Renderer* renderer = NULL;
    SDL_Texture* texture = NULL;
    
    bool m_isRunning = false;
    bool m_use_dod = false;
    bool m_apply_collisions = true;
    bool m_optimizedCollisions = true;
    bool m_drawGrid = false;
    
    // date pt a masura performanta
    float m_fps = 0;
    float m_processInputTime = 0;
    float m_updateTime = 0;
    float m_renderTime = 0;
    Uint64 m_perfStart = 0;

    size_t oop_bytes = 0;
    size_t dod_bytes = 0;
    size_t grid_bytes = 0;
    void calculate_memory();

    
    int m_spawn_quantity = 100;
    const int MAX_ENTITIES = 100000;
    int m_lastUsedId = 0;

    // vector pt modul OOP
    std::vector<FloatingObject*> m_objects;

    // vectori pt modul DOD
    std::vector<int> m_dod_ids;
    std::vector<float> m_dod_pos_x;
    std::vector<float> m_dod_pos_y;
    std::vector<float> m_dod_vel_x;
    std::vector<float> m_dod_vel_y;

    SpatialGrid m_grid;
    int m_gridCellSize = 50;
};