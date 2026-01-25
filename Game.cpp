#include "Game.h"

#include "constants.h"
#include <SDL3/SDL_keycode.h>
#include <iostream>


Game::Game() : m_grid(Constants::g_WINDOW_WIDTH, Constants::g_WINDOW_HEIGHT, m_gridCellSize)
{}

Game::~Game() {
    for (int i = 0; i < m_actors.size(); i++) {
        delete m_actors[i]; // dezaloc memoria
    }
    m_actors.clear();

    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();


    for (SDL_Texture* t: state_textures) {
        SDL_DestroyTexture(t);
    }

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


    SDL_Surface* cyan = IMG_Load("cyan.png"); // incarc textura
    SDL_Surface* white = IMG_Load("white.png"); // incarc textura
    SDL_Surface* red = IMG_Load("red.png"); // incarc textura
    SDL_Surface* green = IMG_Load("green.png"); // incarc textura
    SDL_Surface* yellow = IMG_Load("yellow.png"); // incarc textura
    SDL_Surface* pink = IMG_Load("pink.png"); // incarc textura


    if(!cyan || !white || !red || !green || !yellow || !pink) return false;

    state_textures.resize(NUMBER_OF_STATES);

    SDL_Texture* healthy_texture = SDL_CreateTextureFromSurface(renderer, green); // creez textura din surface
    SDL_Texture* sick_texture = SDL_CreateTextureFromSurface(renderer, red);
    SDL_Texture* doctor_texture = SDL_CreateTextureFromSurface(renderer, white);
    SDL_Texture* healed_texture = SDL_CreateTextureFromSurface(renderer, cyan);
    SDL_Texture* naturally_recovered_texture = SDL_CreateTextureFromSurface(renderer, yellow);
    SDL_Texture* immunized_texture = SDL_CreateTextureFromSurface(renderer, pink);

    if(!healthy_texture || !sick_texture || !doctor_texture || !healed_texture ||
        !naturally_recovered_texture || !immunized_texture)
        return false;

    state_textures[ACTOR_TYPES::SICK] = sick_texture;
    state_textures[ACTOR_TYPES::DOCTOR] = doctor_texture;
    state_textures[ACTOR_TYPES::HEALTHY] = healthy_texture;
    state_textures[ACTOR_TYPES::NATURAL_RECOVERY] = naturally_recovered_texture;
    state_textures[ACTOR_TYPES::HEALED] = healed_texture;
    state_textures[ACTOR_TYPES::IMMUNIZED] = immunized_texture;

    // eliberez surface-ul din memorie
    SDL_DestroySurface(cyan);
    SDL_DestroySurface(red);
    SDL_DestroySurface(green);
    SDL_DestroySurface(white);
    SDL_DestroySurface(yellow);
    SDL_DestroySurface(pink);
    ////////////////


    // initializare spatial grid
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
            case SDL_EVENT_KEY_DOWN: {
                if (event.key.key == SDLK_ESCAPE) { // a apasat ESC
                    m_isRunning = false;
                }
                break;
            }
            default:
                break;
        }
    }
}

void Game::update(float deltaTime) {
    manage_entity_count(); // verifica daca au fost adaugate/eliminate entitati

    // dau resize la grid daca am schimbat dimensiunea unei celule
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

    check_screen_bounds();
    calculate_memory();

    if (m_use_dod) {
        for (int i = 0; i < m_actor_types.size(); i++) {
            if (m_actor_types[i] == ACTOR_TYPES::SICK) {
                if (rand() % 10000 < 1) {
                    m_actor_types[i] = ACTOR_TYPES::NATURAL_RECOVERY; // 0.0001% sansa autovindecare
                }
            }
        }
    }
    else {
        for (int i = 0; i < m_actors.size(); i++) {
            if (m_actors[i]->m_status == ACTOR_TYPES::SICK) {
                if (rand() % 1000 < 1) {
                    m_actors[i]->m_status = ACTOR_TYPES::NATURAL_RECOVERY; // 0.001% sansa autovindecare
                }
            }
        }
    }

}

void Game::render() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    update_imgui();

    if (m_drawGrid) {
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

        // desenez fiecare celula cu linii verticale si orizontale
        for (int x = 0; x <= Constants::g_WINDOW_WIDTH; x += m_gridCellSize) {
            SDL_RenderLine(renderer, x, 0, x, Constants::g_WINDOW_HEIGHT);
        }

        for (int y = 0; y <= Constants::g_WINDOW_HEIGHT; y += m_gridCellSize) {
            SDL_RenderLine(renderer, 0, y, Constants::g_WINDOW_WIDTH, y);
        }
    }

    // prima data dau render la obiecte
    if (m_use_dod) {
        for (int i = 0; i < m_actor_ids.size(); i++) {
            SDL_FRect destRect = {
                m_actor_positions_x[i] - Constants::g_BALL_DIAMETER / 2, // scad raza pentru ca originea este in centru
                m_actor_positions_y[i] - Constants::g_BALL_DIAMETER / 2, // scad raza pentru ca originea este in centru
                (float) Constants::g_BALL_DIAMETER,
                (float) Constants::g_BALL_DIAMETER
            };

            SDL_RenderTexture(renderer, state_textures[m_actor_types[i]], NULL, &destRect);
        }
    }
    else {
        for (int i = 0; i < m_actors.size(); i++) {
            m_actors[i]->render();
        }
    }

    // vreau ca fereastra pentru imgui sa fie mereu deasupra
    ImGui::Render();
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);

    SDL_RenderPresent(renderer);
}

void Game::apply_physics(float deltaTime) {
    deltaTime *= Constants::g_SIMULATION_SPEED;

    if (m_use_dod) {
        for(int i = 0; i < m_actor_ids.size(); i++) {
            m_actor_positions_x[i] += m_actor_velocities_x[i] * deltaTime;
            m_actor_positions_y[i] += m_actors_velocities_y[i] * deltaTime;
        }
    }
    else {
        for(int i = 0; i < m_actors.size(); i++) {
            m_actors[i]->update(deltaTime);
        }
    }
}

void Game::enforce_boundaries(float& x, float& y, float& vx, float& vy) {
    float radius = Constants::g_BALL_DIAMETER / 2.0f;
    float energy_loss = -1.0f; // vreau ca actorii sa mearga incontinuu

    if (y + radius > Constants::g_WINDOW_HEIGHT) // coliziune cu marginea de jos
    {
        y = Constants::g_WINDOW_HEIGHT - radius;
        vy *= energy_loss;
    }
    else if (y - radius < 0) // coliziune cu marginea de sus
    {
        y = radius;
        vy *= energy_loss;
    }

    if (x + radius > Constants::g_WINDOW_WIDTH) { // coliziune cu marginea din dreapta
        x = Constants::g_WINDOW_WIDTH - radius;
        vx *= energy_loss;
    }
    else if (x - radius < 0) // coliziune cu marginea din stanga
    {
        x = radius;
        vx *= energy_loss;
    }
}

void Game::check_screen_bounds() {
    if(m_use_dod) {
        for(int i = 0; i < m_actor_ids.size(); i++) {
            enforce_boundaries(m_actor_positions_x[i], m_actor_positions_y[i], m_actor_velocities_x[i], m_actors_velocities_y[i]);
        }
    }
    else {
        for(int i = 0; i < m_actors.size(); i++) {
            float& x = m_actors[i]->m_position.x;
            float& y = m_actors[i]->m_position.y;
            float& vx = m_actors[i]->m_velocity.x;
            float& vy = m_actors[i]->m_velocity.y;

            enforce_boundaries(x, y, vx, vy);
        }
    }
}

void Game::update_health_status(int i, int j) {
    int odd = rand() % 100; // generez o sansa

    if (m_use_dod) {
        if (m_actor_types[i] == ACTOR_TYPES::NATURAL_RECOVERY || m_actor_types[j] == ACTOR_TYPES::NATURAL_RECOVERY) {
            return;
        }
        if (m_actor_types[i] == ACTOR_TYPES::IMMUNIZED || m_actor_types[j] == ACTOR_TYPES::IMMUNIZED) {
            return;
        }

        // coliziune SICK cu DOCTOR
        if ((m_actor_types[i] == ACTOR_TYPES::DOCTOR && m_actor_types[j] == ACTOR_TYPES::SICK) ||
            (m_actor_types[i] == ACTOR_TYPES::SICK && m_actor_types[j] == ACTOR_TYPES::DOCTOR))
        {
            int doctor_idx = (m_actor_types[i] == ACTOR_TYPES::DOCTOR) ? i : j;
            int sick_idx = (m_actor_types[i] == ACTOR_TYPES::SICK) ? i : j;

            if (odd < 1) {
                // 1% se imbolnaveste doctorul
                m_actor_types[doctor_idx] = ACTOR_TYPES::SICK;
            }
            else if (odd < 86) {
                // 85% sick este tratat
                m_actor_types[sick_idx] = ACTOR_TYPES::HEALED;
            }
        }
        // coliziune HEALTHY cu DOCTOR
        else if ((m_actor_types[i] == ACTOR_TYPES::DOCTOR && m_actor_types[j] == ACTOR_TYPES::HEALTHY) ||
                 (m_actor_types[i] == ACTOR_TYPES::HEALTHY && m_actor_types[j] == ACTOR_TYPES::DOCTOR))
        {
            int healthy_idx = (m_actor_types[i] == ACTOR_TYPES::HEALTHY) ? i : j;
            m_actor_types[healthy_idx] = ACTOR_TYPES::IMMUNIZED; // se imunizeaza actorul HEALTHY
        }
        // coliziune HEALTHY cu SICK
        else if ((m_actor_types[i] == ACTOR_TYPES::SICK && m_actor_types[j] == ACTOR_TYPES::HEALTHY) ||
                 (m_actor_types[i] == ACTOR_TYPES::HEALTHY && m_actor_types[j] == ACTOR_TYPES::SICK))
        {
            int healthy_idx = (m_actor_types[i] == ACTOR_TYPES::HEALTHY) ? i : j;

            if (odd < 80) {
                // 80% actorul HEALTHY se imbolnaveste
                m_actor_types[healthy_idx] = ACTOR_TYPES::SICK;
            }
        }
    }
    else {
        Actor* obj1 = m_actors[i];
        Actor* obj2 = m_actors[j];

        if (obj1->m_status == ACTOR_TYPES::NATURAL_RECOVERY || obj2->m_status == ACTOR_TYPES::NATURAL_RECOVERY) {
            return;
        }
        if (obj1->m_status == ACTOR_TYPES::IMMUNIZED || obj2->m_status == ACTOR_TYPES::IMMUNIZED) {
            return;
        }

        // coliziune SICK cu DOCTOR
        if ((obj1->m_status == ACTOR_TYPES::DOCTOR && obj2->m_status == ACTOR_TYPES::SICK) ||
            (obj1->m_status == ACTOR_TYPES::SICK && obj2->m_status == ACTOR_TYPES::DOCTOR))
        {
            Actor* doctor = (obj1->m_status == ACTOR_TYPES::DOCTOR) ? obj1 : obj2;
            Actor* sick = (obj1->m_status == ACTOR_TYPES::SICK) ? obj1 : obj2;

            if (odd < 1) {
                // 1% se imbolnaveste doctorul
                doctor->m_status = ACTOR_TYPES::SICK;
            }
            else if (odd < 86) {
                // 85% sick este tratat
                sick->m_status = ACTOR_TYPES::HEALED;
            }
        }
        // coliziune HEALTHY cu DOCTOR
        else if ((obj1->m_status == ACTOR_TYPES::DOCTOR && obj2->m_status == ACTOR_TYPES::HEALTHY) ||
                 (obj1->m_status == ACTOR_TYPES::HEALTHY && obj2->m_status == ACTOR_TYPES::DOCTOR))
        {
            Actor* healthy = (obj1->m_status == ACTOR_TYPES::HEALTHY) ? obj1 : obj2;
            healthy->m_status = ACTOR_TYPES::IMMUNIZED;
        }
        // coliziune HEALTHY cu SICK
        else if ((obj1->m_status == ACTOR_TYPES::SICK && obj2->m_status == ACTOR_TYPES::HEALTHY) ||
                 (obj1->m_status == ACTOR_TYPES::HEALTHY && obj2->m_status == ACTOR_TYPES::SICK))
        {
            Actor* healthy = (obj1->m_status == ACTOR_TYPES::HEALTHY) ? obj1 : obj2;

            if (odd < 80) {
                // 80% actorul HEALTHY se imbolnaveste
                healthy->m_status = ACTOR_TYPES::SICK;
            }
        }
    }
}



void Game::optimized_resolve_collisions() {
    float radius = Constants::g_BALL_DIAMETER / 2.0f;
    int count = m_use_dod ? m_actor_ids.size() : m_actors.size();

    // recalculez grid-ul
    m_grid.clear();
    for (int i = 0; i < count; ++i) {
        float x = m_use_dod ? m_actor_positions_x[i] : m_actors[i]->m_position.x;
        float y = m_use_dod ? m_actor_positions_y[i] : m_actors[i]->m_position.y;
        m_grid.insert(x, y, i);
    }

    for (int i = 0; i < count; ++i) {
        float x1, y1;

        if (m_use_dod) {
            x1 = m_actor_positions_x[i];
            y1 = m_actor_positions_y[i];
        }
        else {
            x1 = m_actors[i]->m_position.x;
            y1 = m_actors[i]->m_position.y;
        }

        int col = (int)(x1 / m_gridCellSize);
        int row = (int)(y1 / m_gridCellSize);

        // edge case: actori care sunt intre 2 celule din grid
        // solutie: verific vecinii unei celule
        for (int dy = -1; dy <= 1; dy++) {
            for (int dx = -1; dx <= 1; dx++) {
                std::vector<int> cell = m_grid.get_cell(col + dx, row + dy);

                for (int j: cell) { // j = vectori de indecsi
                    if (j <= i) {
                        continue;
                    }

                    if (m_use_dod)
                    {
                        if (MathUtils::circles_overlap(m_actor_positions_x[i], m_actor_positions_y[i], m_actor_positions_x[j], m_actor_positions_y[j], radius))
                        {
                            MathUtils::resolve_elastic_collision(
                                m_actor_positions_x[i], m_actor_positions_y[i], m_actor_velocities_x[i], m_actors_velocities_y[i],
                                m_actor_positions_x[j], m_actor_positions_y[j], m_actor_velocities_x[j], m_actors_velocities_y[j],
                                radius
                            );

                            update_health_status(i, j);
                        }
                    }
                    else {
                        Actor* o1 = m_actors[i];
                        Actor* o2 = m_actors[j];

                        if (MathUtils::circles_overlap(o1->m_position.x, o1->m_position.y, o2->m_position.x, o2->m_position.y, radius))
                        {
                            MathUtils::resolve_elastic_collision(
                                o1->m_position.x, o1->m_position.y, o1->m_velocity.x, o1->m_velocity.y,
                                o2->m_position.x, o2->m_position.y, o2->m_velocity.x, o2->m_velocity.y,
                                radius
                            );

                            update_health_status(i, j);
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
        for (int i = 0; i < m_actor_ids.size(); i++) {
            float x1 = m_actor_positions_x[i];
            float y1 = m_actor_positions_y[i];
            float vx1 = m_actor_velocities_x[i];
            float vy1 = m_actors_velocities_y[i];

            for (int j = i + 1; j < m_actor_ids.size();  j++) {
                float x2 = m_actor_positions_x[j];
                float y2 = m_actor_positions_y[j];
                float vx2 = m_actor_velocities_x[j];
                float vy2 = m_actors_velocities_y[j];

                if (MathUtils::circles_overlap(x1, y1, x2, y2, radius)) {
                    MathUtils::resolve_elastic_collision(x1, y1, vx1, vy1, x2, y2, vx2, vy2, radius);

                    update_health_status(i, j);
                }
            }
        }
    }
    else {
        for (int i = 0; i < m_actors.size(); i++) {
            float& x1 = m_actors[i]->m_position.x;
            float& y1 = m_actors[i]->m_position.y;
            float& vx1 = m_actors[i]->m_velocity.x;
            float& vy1 = m_actors[i]->m_velocity.y;

            for (int j = i + 1; j < m_actor_ids.size();  j++) {
                float& x2 = m_actors[j]->m_position.x;
                float& y2 = m_actors[j]->m_position.y;
                float& vx2 = m_actors[j]->m_velocity.x;
                float& vy2 = m_actors[j]->m_velocity.y;

                if (MathUtils::circles_overlap(x1, y1, x2, y2, radius)) {
                    MathUtils::resolve_elastic_collision(x1, y1, vx1, vy1, x2, y2, vx2, vy2, radius);
                    update_health_status(i, j);
                }
            }
        }
    }
}


void Game::manage_entity_count() {
    while (m_actors.size() < m_spawn_quantity) {
        int id = ++m_lastUsedId;
        int status;
        int chance = rand() % 100;

        if (chance < 10) { // 10% sansa sa am un doctor
            status = ACTOR_TYPES::DOCTOR;
        }
        else {
            // daca nu am doctor, generez random HEALTHY sau SICK
            status = (rand() % 2) + 1;
        }

        Actor* obj = new Actor(state_textures, renderer, id, status);
        m_actors.push_back(obj);

        m_actor_ids.push_back(id);
        m_actor_positions_x.push_back(obj->m_position.x);
        m_actor_positions_y.push_back(obj->m_position.y);
        m_actor_velocities_x.push_back(obj->m_velocity.x);
        m_actors_velocities_y.push_back(obj->m_velocity.y);
        m_actor_types.push_back(status);
    }

    while (m_actors.size() > m_spawn_quantity) {
        delete m_actors.back();
        m_actors.pop_back();

        m_actor_ids.pop_back();
        m_actor_positions_x.pop_back();
        m_actor_positions_y.pop_back();
        m_actor_velocities_x.pop_back();
        m_actors_velocities_y.pop_back();
        m_actor_types.pop_back();
    }
}

void Game::sync_state_to_dod() {
    for (size_t i = 0; i < m_actors.size(); ++i) {
        m_actor_positions_x[i] = m_actors[i]->m_position.x;
        m_actor_positions_y[i] = m_actors[i]->m_position.y;
        m_actor_velocities_x[i] = m_actors[i]->m_velocity.x;
        m_actors_velocities_y[i] = m_actors[i]->m_velocity.y;
        m_actor_types[i] = m_actors[i]->m_status;
    }
}

void Game::sync_state_to_oop() {
    for (size_t i = 0; i < m_actors.size(); ++i) {
        m_actors[i]->m_position.x = m_actor_positions_x[i];
        m_actors[i]->m_position.y = m_actor_positions_y[i];
        m_actors[i]->m_velocity.x = m_actor_velocities_x[i];
        m_actors[i]->m_velocity.y = m_actors_velocities_y[i];
        m_actors[i]->m_status = m_actor_types[i];
    }
}

void Game::calculate_memory() {
    oop_bytes = m_actors.capacity() * sizeof(Actor*) + m_actors.size() * sizeof(Actor);
    dod_bytes = m_actor_ids.capacity() * sizeof(int) + (m_actor_positions_x.capacity() + m_actor_positions_y.capacity() + m_actor_velocities_x.capacity() + m_actors_velocities_y.capacity()) * sizeof(float);
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

    // numar cati DOCTORS, HEALTHY, SICK, ... actori am
    std::vector<int> counters(NUMBER_OF_STATES);
    if (m_use_dod) {
        for (int i = 0; i < m_actor_types.size(); i++) {
            counters[m_actor_types[i]]++;
        }
    }
    else {
        for (int i = 0; i < m_actors.size(); i++) {
            counters[m_actors[i]->m_status]++;
        }
    }

    ImGui::Separator();

    ImGui::Text("Simulation data:");
    ImGui::Text("Doctors: %d", counters[ACTOR_TYPES::DOCTOR]);
    ImGui::Text("Healthy: %d", counters[ACTOR_TYPES::HEALTHY]);
    ImGui::Text("Natural recovery: %d", counters[ACTOR_TYPES::NATURAL_RECOVERY]);
    ImGui::Text("Vaccinated: %d", counters[ACTOR_TYPES::IMMUNIZED]);
    ImGui::Text("Healed: %d", counters[ACTOR_TYPES::HEALED]);
    ImGui::Text("Sick: %d", counters[ACTOR_TYPES::SICK]);

    ImGui::Separator();

    ImGui::Text("Memory Usage:");
    ImGui::Text("OOP Memory usage: %.3f KB", oop_bytes / 1024.0f);
    ImGui::Text("DOD Memory usage: %.3f KB", dod_bytes / 1024.0f);
    ImGui::Text("Grid Memory usage: %.3f KB", grid_bytes / 1024.0f);
    ImGui::Separator();
    
    ImGui::Separator();
    ImGui::SliderInt("Entities", &m_spawn_quantity, 0, MAX_ENTITIES);
    ImGui::SliderInt("Balls diameter", &Constants::g_BALL_DIAMETER, 1, 50);
    ImGui::SliderFloat("Simulation speed", &Constants::g_SIMULATION_SPEED, 0.0f, 10.0f);

    if (ImGui::RadioButton("OOP Mode", !m_use_dod)) {
        if (m_use_dod) {
            sync_state_to_oop();
        }
        m_use_dod = false;
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("DOD Mode", m_use_dod)) {
        if (!m_use_dod) {
            sync_state_to_dod();
        }
        m_use_dod = true;
    }

    ImGui::Checkbox("Collisions", &m_apply_collisions);
    ImGui::Checkbox("Show grid", &m_drawGrid);
    ImGui::Checkbox("Optimized collisions", &m_optimizedCollisions);

    ImGui::Separator();
    ImGui::Text("Grid cell size");

    // 200 -> gcd rezolutie ecran (vreau celule care divid exact ecranul)
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
