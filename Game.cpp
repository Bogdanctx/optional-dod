#include "Game.h"

#include "constants.h"
#include <SDL3/SDL_keycode.h>
#include <iostream>


Game::Game() : m_grid(Constants::g_WINDOW_WIDTH, Constants::g_WINDOW_HEIGHT, m_gridCellSize)
{

}

Game::~Game() {
    for (int i = 0; i < m_objects.size(); i++) {
        delete m_objects[i]; // dezaloc memoria
    }
    m_objects.clear();

    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}


bool Game::init() {
    SDL_Init(SDL_INIT_VIDEO); // initializare sdl

    window = SDL_CreateWindow("Proiect DOD", Constants::g_WINDOW_WIDTH, Constants::g_WINDOW_HEIGHT, SDL_WINDOW_OPENGL);

    if (!window) return false;

    renderer = SDL_CreateRenderer(window, NULL);

    if (!renderer) return false;

    // initializare imgui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext(); // creare fereastra imgui
    ImGui::StyleColorsDark(); // dark mode
    ImGui_ImplSDL3_InitForSDLRenderer(window, renderer); // initializare imgui pentru sdl
    ImGui_ImplSDLRenderer3_Init(renderer); // initializare imgui pentru renderer


    SDL_Surface* surface = IMG_Load("ball.png"); // incarc textura
    
    if(!surface) return false;

    texture = SDL_CreateTextureFromSurface(renderer, surface); // creez textura din surface
    
    if(!texture) return false;
    
    SDL_DestroySurface(surface); // eliberez surface-ul din memorie pentru ca nu mai am nevoie de el


    m_grid = SpatialGrid(Constants::g_WINDOW_WIDTH, Constants::g_WINDOW_HEIGHT, m_gridCellSize);
    m_isRunning = true;

    return true;
}

void Game::run_loop() {
    Uint64 lastTime = SDL_GetPerformanceCounter();

    while (m_isRunning) {
        Uint64 frameStart = SDL_GetPerformanceCounter();
        float deltaTime = (float)((frameStart - lastTime) * 1000 / (double)SDL_GetPerformanceFrequency()) / 1000.0f;
        lastTime = frameStart;
        m_fps = 1.0f / deltaTime;

        Chronometer::start();
        process_input();
        m_processInputTime = Chronometer::stop() * 1000.0 / SDL_GetPerformanceFrequency();

        Chronometer::start();
        update(deltaTime);
        m_updateTime = Chronometer::stop() * 1000.0 / SDL_GetPerformanceFrequency();

        Chronometer::start();
        render();
        m_renderTime = Chronometer::stop() * 1000.0 / SDL_GetPerformanceFrequency();
    }
}

void Game::process_input() {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        ImGui_ImplSDL3_ProcessEvent(&event); // trimite evenimentul catre ImGui

        switch (event.type) {
            case SDL_EVENT_QUIT: { // fereastra este inchisa
                m_isRunning = false;
                break;
            }
            default:
                break;
        }
    }
}

void Game::update(float deltaTime) {
    manage_entity_count();

    if(m_grid.get_cell_size() != m_gridCellSize) {
        m_grid.resize(Constants::g_WINDOW_WIDTH, Constants::g_WINDOW_HEIGHT, m_gridCellSize);
    }

    apply_physics(deltaTime);

    if(m_apply_collisions) {
        if (m_optimizedCollisions) {
            optimized_resolve_collisions();
        }
        else {
            naive_resolve_collisions();
        }
    }

    enforce_boundaries();
    calculate_memory();
}

void Game::render() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    update_imgui();

    if (m_drawGrid) {
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

        // Ca sa desenez fiecare celula prima data trasez linii verticale si dupa linii orizontale
        for (int x = 0; x <= Constants::g_WINDOW_WIDTH; x += m_gridCellSize) {
            SDL_RenderLine(renderer, x, 0, x, Constants::g_WINDOW_HEIGHT);
        }
        
        for (int y = 0; y <= Constants::g_WINDOW_HEIGHT; y += m_gridCellSize) {
            SDL_RenderLine(renderer, 0, y, Constants::g_WINDOW_WIDTH, y);
        }
    }

    // prima data dau render la obiecte
    int numberOfObjects = m_objects.size();
    if (m_use_dod) {
        for (int i = 0; i < numberOfObjects; i++) {
            SDL_FRect destRect = {
                m_dod_pos_x[i] - Constants::g_BALL_DIAMETER / 2, // scad radius pentru ca originea este in centru
                m_dod_pos_y[i] - Constants::g_BALL_DIAMETER / 2, // scad radius pentru ca originea este in centru
                (float) Constants::g_BALL_DIAMETER,
                (float) Constants::g_BALL_DIAMETER
            };
            SDL_RenderTexture(renderer, texture, NULL, &destRect);
        }
    }
    else {
        for (int i = 0; i < numberOfObjects; i++) {
            m_objects[i]->render();
        }
    }

    // vreau ca fereastra pentru imgui sa fie mereu deasupra
    ImGui::Render();
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);

    SDL_RenderPresent(renderer);
}

void Game::apply_physics(float deltaTime) {
    float dt = deltaTime * FloatingObject::speed_multiplier;

    if (m_use_dod) {
        for(int i = 0; i < m_dod_ids.size(); i++) {
            m_dod_vel_y[i] += Constants::g_GRAVITY * dt;

            m_dod_pos_x[i] += m_dod_vel_x[i] * dt;
            m_dod_pos_y[i] += m_dod_vel_y[i] * dt;
        }
    }
    else {
        for(int i = 0; i < m_objects.size(); i++) { 
            m_objects[i]->update(dt);
        }
    }
}

void Game::check_screen_bounds(float& x, float& y, float& vx, float& vy) {
    float radius = Constants::g_BALL_DIAMETER / 2.0f;
    float energy_loss = -0.5f;
    
    if (y + radius > Constants::g_WINDOW_HEIGHT) 
    {
        y = Constants::g_WINDOW_HEIGHT - radius;
        vy *= energy_loss;
    } 
    else if (y - radius < 0)
    {
        y = radius;
        vy *= energy_loss;
    }
    
    if (x + radius > Constants::g_WINDOW_WIDTH) {
        x = Constants::g_WINDOW_WIDTH - radius;
        vx *= energy_loss;
    } 
    else if (x - radius < 0) 
    {
        x = radius;
        vx *= energy_loss;
    }
}

void Game::enforce_boundaries() {
    if(m_use_dod) {
        for(int i = 0; i < m_dod_ids.size(); i++) {
            check_screen_bounds(m_dod_pos_x[i], m_dod_pos_y[i], m_dod_vel_x[i], m_dod_vel_y[i]);
        }
    }
    else {
        for(int i = 0; i < m_objects.size(); i++) {
            float& x = m_objects[i]->m_position.x;
            float& y = m_objects[i]->m_position.y;
            float& vx = m_objects[i]->m_velocity.x;
            float& vy = m_objects[i]->m_velocity.y;

            check_screen_bounds(x, y, vx, vy);
        }
    }
}

void Game::optimized_resolve_collisions() {
    float radius = Constants::g_BALL_DIAMETER / 2.0f;
    int count = m_use_dod ? m_dod_ids.size() : m_objects.size();

    m_grid.clear();
    for (int i = 0; i < count; ++i) {
        float x = m_use_dod ? m_dod_pos_x[i] : m_objects[i]->m_position.x;
        float y = m_use_dod ? m_dod_pos_y[i] : m_objects[i]->m_position.y;
        m_grid.insert(x, y, i);
    }

    for (int i = 0; i < count; ++i) {
        float x1, y1;
        
        if (m_use_dod) { x1 = m_dod_pos_x[i]; y1 = m_dod_pos_y[i]; }
        else { x1 = m_objects[i]->m_position.x; y1 = m_objects[i]->m_position.y; }

        int col = (int)(x1 / m_gridCellSize);
        int row = (int)(y1 / m_gridCellSize);

        for (int dy = -1; dy <= 1; dy++) {
            for (int dx = -1; dx <= 1; dx++) {
                std::vector<int> cell = m_grid.get_cell(col + dx, row + dy);
                
                for (int j : cell) {
                    if (j <= i) continue;

                    if (m_use_dod) {
                        if (MathUtils::circles_overlap(x1, y1, m_dod_pos_x[j], m_dod_pos_y[j], radius)) {
                            MathUtils::resolve_elastic_collision(
                                m_dod_pos_x[i], m_dod_pos_y[i], m_dod_vel_x[i], m_dod_vel_y[i],
                                m_dod_pos_x[j], m_dod_pos_y[j], m_dod_vel_x[j], m_dod_vel_y[j],
                                radius
                            );

                            x1 = m_dod_pos_x[i];
                            y1 = m_dod_pos_y[i];
                        }
                    } else {
                        FloatingObject* o1 = m_objects[i];
                        FloatingObject* o2 = m_objects[j];
                        if (MathUtils::circles_overlap(o1->m_position.x, o1->m_position.y, o2->m_position.x, o2->m_position.y, radius)) {
                            MathUtils::resolve_elastic_collision(
                                o1->m_position.x, o1->m_position.y, o1->m_velocity.x, o1->m_velocity.y,
                                o2->m_position.x, o2->m_position.y, o2->m_velocity.x, o2->m_velocity.y,
                                radius
                            );
                            x1 = o1->m_position.x;
                            y1 = o1->m_position.y;
                        }
                    }
                }
            }
        }
    }
}

void Game::naive_resolve_collisions() {
    float radius = Constants::g_BALL_DIAMETER / 2;

    if (m_use_dod) {
        for (int i = 0; i < m_dod_ids.size(); i++) {
            float x1 = m_dod_pos_x[i];
            float y1 = m_dod_pos_y[i];
            float vx1 = m_dod_vel_x[i];
            float vy1 = m_dod_vel_y[i];

            for (int j = i + 1; j < m_dod_ids.size();  j++) {
                float x2 = m_dod_pos_x[j];
                float y2 = m_dod_pos_y[j];
                float vx2 = m_dod_vel_x[j];
                float vy2 = m_dod_vel_y[j];

                if (MathUtils::circles_overlap(x1, y1, x2, y2, radius)) {
                    MathUtils::resolve_elastic_collision(x1, y1, vx1, vy1, x2, y2, vx2, vy2, radius);
                }
            }
        }
    }
    else {
        for (int i = 0; i < m_objects.size(); i++) {
            float& x1 = m_objects[i]->m_position.x;
            float& y1 = m_objects[i]->m_position.y;
            float& vx1 = m_objects[i]->m_velocity.x;
            float& vy1 = m_objects[i]->m_velocity.y;

            for (int j = i + 1; j < m_dod_ids.size();  j++) {
                float& x2 = m_objects[j]->m_position.x;
                float& y2 = m_objects[j]->m_position.y;
                float& vx2 = m_objects[j]->m_velocity.x;
                float& vy2 = m_objects[j]->m_velocity.y;

                if (MathUtils::circles_overlap(x1, y1, x2, y2, radius)) {
                    MathUtils::resolve_elastic_collision(x1, y1, vx1, vy1, x2, y2, vx2, vy2, radius);
                }
            }
        }
    }
}


void Game::manage_entity_count() {
    while (m_objects.size() < m_spawn_quantity) {
        int id = ++m_lastUsedId;
        FloatingObject* obj = new FloatingObject(texture, renderer, id);
        m_objects.push_back(obj);
        
        m_dod_ids.push_back(id);
        m_dod_pos_x.push_back(obj->m_position.x);
        m_dod_pos_y.push_back(obj->m_position.y);
        m_dod_vel_x.push_back(obj->m_velocity.x);
        m_dod_vel_y.push_back(obj->m_velocity.y);
    }

    while (m_objects.size() > m_spawn_quantity) {
        delete m_objects.back();
        m_objects.pop_back();

        m_dod_ids.pop_back();
        m_dod_pos_x.pop_back();
        m_dod_pos_y.pop_back();
        m_dod_vel_x.pop_back();
        m_dod_vel_y.pop_back();
    }
}

void Game::sync_state_to_dod() {
    for (size_t i = 0; i < m_objects.size(); ++i) {
        m_dod_pos_x[i] = m_objects[i]->m_position.x;
        m_dod_pos_y[i] = m_objects[i]->m_position.y;
        m_dod_vel_x[i] = m_objects[i]->m_velocity.x;
        m_dod_vel_y[i] = m_objects[i]->m_velocity.y;
    }
}

void Game::sync_state_to_oop() {
    for (size_t i = 0; i < m_objects.size(); ++i) {
        m_objects[i]->m_position.x = m_dod_pos_x[i];
        m_objects[i]->m_position.y = m_dod_pos_y[i];
        m_objects[i]->m_velocity.x = m_dod_vel_x[i];
        m_objects[i]->m_velocity.y = m_dod_vel_y[i];
    }
}

void Game::calculate_memory() {
    oop_bytes = m_objects.capacity() * sizeof(FloatingObject*) + m_objects.size() * sizeof(FloatingObject);
    dod_bytes = m_dod_ids.capacity() * sizeof(int) + (m_dod_pos_x.capacity() + m_dod_pos_y.capacity() + m_dod_vel_x.capacity() + m_dod_vel_y.capacity()) * sizeof(float);
    grid_bytes = m_grid.get_memory_usage();
}

void Game::update_imgui() 
{
    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Panel");
    ImGui::Text("FPS: %.1f", m_fps);
    ImGui::Text("Process input time: %.3f", m_processInputTime);
    ImGui::Text("Update time: %.3f", m_updateTime);
    ImGui::Text("Render time: %.3f", m_renderTime);
    ImGui::Separator();

    ImGui::Text("Memory Usage:");
    ImGui::Text("OOP Memory usage: %.3f KB", oop_bytes / 1024.0f);
    ImGui::Text("DOD Memory usage: %.3f KB", dod_bytes / 1024.0f);
    ImGui::Text("Grid Memory usage: %.3f KB", grid_bytes / 1024.0f);
    ImGui::Separator();
    
    ImGui::Separator();
    ImGui::SliderFloat("Simulation speed", &FloatingObject::speed_multiplier, 0.0f, 50.0f);
    ImGui::SliderInt("Entities", &m_spawn_quantity, 0, MAX_ENTITIES);
    ImGui::SliderFloat("Gravity", &Constants::g_GRAVITY, 0, 5000.0f);
    ImGui::SliderInt("Balls diameter", &Constants::g_BALL_DIAMETER, 1, 50);

    if (ImGui::RadioButton("OOP Mode", !m_use_dod)) {
        if (m_use_dod) sync_state_to_oop();
        m_use_dod = false;
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("DOD Mode", m_use_dod)) {
        if (!m_use_dod) sync_state_to_dod();
        m_use_dod = true;
    }

    ImGui::Checkbox("Collisions", &m_apply_collisions);
    ImGui::Checkbox("Show grid", &m_drawGrid);
    ImGui::Checkbox("Optimized collisions", &m_optimizedCollisions);

    ImGui::Separator();
    ImGui::Text("Grid cell size");

    std::vector divisors = {200, 50, 40, 25, 20, 10, 8, 5, 4, 2, 1};

    if (m_gridCellSize <= Constants::g_BALL_DIAMETER) {
        for (int i = divisors.size() - 1; i >= 0; i--) {
            if (divisors[i] > Constants::g_BALL_DIAMETER) {
                m_gridCellSize = divisors[i];
                break;
            }
        }
    }

    for (int divisor: divisors) {
        if (divisor > Constants::g_BALL_DIAMETER) {
            std::string d = std::to_string(divisor);
            ImGui::RadioButton(d.c_str(), &m_gridCellSize, divisor);
            ImGui::SameLine();
        }
    }

    ImGui::End();
}
