#include "Game.h"

#include <charconv>

#include "utils.h"
#include <SDL3/SDL_keycode.h>
#include <iostream>

Uint64 Game::start;

Game::~Game() {
    for (int i = 0; i < m_objects.size(); i++) {
        delete m_objects[i]; // dezaloc memoria
    }
}


bool Game::init() {
    SDL_Init(SDL_INIT_VIDEO); // initializare sdl

    window = SDL_CreateWindow("Proiect DOD", Utils::g_WINDOW_WIDTH, Utils::g_WINDOW_HEIGHT, SDL_WINDOW_OPENGL);

    if (window == NULL) {
        return false;
    }

    renderer = SDL_CreateRenderer(window, NULL);

    if (renderer == NULL) {
        return false;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext(); // creare fereastra imgui
    ImGui::StyleColorsDark(); // dark mode
    ImGui_ImplSDL3_InitForSDLRenderer(window, renderer); // initializare imgui pentru sdl
    ImGui_ImplSDLRenderer3_Init(renderer); // initializare imgui pentru renderer

    SDL_Surface* surface = IMG_Load("ball.png"); // incarc textura
    texture = SDL_CreateTextureFromSurface(renderer, surface); // creez textura din surface
    SDL_DestroySurface(surface); // eliberez surface-ul din memorie pentru ca nu mai am nevoie de el

    m_gridColumns = (Utils::g_WINDOW_WIDTH + GRID_CELL_SIZE - 1) / GRID_CELL_SIZE;
    m_gridRows = (Utils::g_WINDOW_HEIGHT + GRID_CELL_SIZE - 1) / GRID_CELL_SIZE;

    m_grid.resize(m_gridRows * m_gridColumns);

    m_isRunning = true;
    return true;
}



int Game::get_grid_cell(float x, float y) {
    int column = (int)(x / GRID_CELL_SIZE);
    int row = (int)(y / GRID_CELL_SIZE);

    if (column < 0) {
        column = 0;
    }
    else if (column >= m_gridColumns) {
        column = m_gridColumns - 1;
    }

    if (row < 0) {
        row = 0;
    }
    else if (row >= m_gridRows) {
        row = m_gridRows - 1;
    }

    int cell_index = row * m_gridColumns + column;
    return cell_index;
}



void Game::update_grid() {
    m_gridColumns = (Utils::g_WINDOW_WIDTH + GRID_CELL_SIZE - 1) / GRID_CELL_SIZE;
    m_gridRows = (Utils::g_WINDOW_HEIGHT + GRID_CELL_SIZE - 1) / GRID_CELL_SIZE;

    if (m_gridColumns * m_gridRows != m_grid.size()) {
        m_grid.resize(m_gridRows * m_gridColumns);
    }

    for (std::vector<int>& cell: m_grid) {
        cell.clear();
    }

    // numarul de entitati de pe ecran
    int n = m_floatingObjects_id.size();

    if (m_use_dod) {
        for (int i = 0; i < n; i++) {
            int cell_index = get_grid_cell(m_floatingObjects_position_x[i], m_floatingObjects_position_y[i]);
            m_grid[cell_index].push_back(i);
        }
    }
    else {
        for (int i = 0; i < n; i++) {
            int cell_index = get_grid_cell(m_objects[i]->m_position.x, m_objects[i]->m_position.y);
            m_grid[cell_index].push_back(i);
        }
    }
}

// Coliziuni pt DOD
void Game::dod_check_collisions_grid() {
    int numberOfObjects = m_floatingObjects_id.size();

    for (int i = 0; i < numberOfObjects; i++) {
        float b1_pos_x = m_floatingObjects_position_x[i];
        float b1_pos_y = m_floatingObjects_position_y[i];

        int col_i = (int)(b1_pos_x / GRID_CELL_SIZE);
        int row_i = (int)(b1_pos_y / GRID_CELL_SIZE);

        const int dxdy[][2] = {
            {-1, -1}, {0, -1}, {1, -1},
            {-1, 0}, {0, 0}, {1, 0},
            {-1, 1}, {0, 1}, {1, 1}
        };

        for (int d = 0; d < 9; d++) {
            int neighbor_col = col_i + dxdy[d][0];
            int neighbor_row = row_i + dxdy[d][1];

            if (neighbor_col >= 0 && neighbor_col < m_gridColumns && neighbor_row >= 0 && neighbor_row < m_gridRows) {

                int neighbor_cell_index = neighbor_row * m_gridColumns + neighbor_col;

                for (int j : m_grid[neighbor_cell_index]) {
                    if (j <= i) {
                        continue;
                    }

                    float b2_pos_x = m_floatingObjects_position_x[j];
                    float b2_pos_y = m_floatingObjects_position_y[j];

                    if (circles_overlap(b1_pos_x, b1_pos_y, b2_pos_x, b2_pos_y)) {
                        float distance = sqrtf((b1_pos_x - b2_pos_x) * (b1_pos_x - b2_pos_x) + (b1_pos_y - b2_pos_y) * (b1_pos_y - b2_pos_y));
                        float overlap = 0.5f * (m_floatingObject_radius + m_floatingObject_radius - distance);

                        float nx = (b2_pos_x - b1_pos_x) / distance;
                        float ny = (b2_pos_y - b1_pos_y) / distance;

                        m_floatingObjects_position_x[i] -= overlap * nx;
                        m_floatingObjects_position_y[i] -= overlap * ny;
                        m_floatingObjects_position_x[j] += overlap * nx;
                        m_floatingObjects_position_y[j] += overlap * ny;

                        float tx = -ny;
                        float ty = nx;

                        float dptan1 = m_floatingObjects_velocity_x[i] * tx + m_floatingObjects_velocity_y[i] * ty;
                        float dptan2 = m_floatingObjects_velocity_x[j] * tx + m_floatingObjects_velocity_y[j] * ty;

                        float dpnorm1 = m_floatingObjects_velocity_x[i] * nx + m_floatingObjects_velocity_y[i] * ny;
                        float dpnorm2 = m_floatingObjects_velocity_x[j] * nx + m_floatingObjects_velocity_y[j] * ny;

                        float m1 = dpnorm2;
                        float m2 = dpnorm1;

                        m_floatingObjects_velocity_x[i] = tx * dptan1 + nx * m1;
                        m_floatingObjects_velocity_y[i] = ty * dptan1 + ny * m1;
                        m_floatingObjects_velocity_x[j] = tx * dptan2 + nx * m2;
                        m_floatingObjects_velocity_y[j] = ty * dptan2 + ny * m2;
                    }
                }
            }
        }
    }
}


// Coliziuni pentru modul OOP
void Game::check_collisions_grid() {
    update_grid();

    if (m_use_dod) {
        dod_check_collisions_grid();
        return;
    }

    int numberOfObjects = m_floatingObjects_id.size();

    for (int i = 0; i < numberOfObjects; i++) {
        float b1_pos_x = m_objects[i]->m_position.x;
        float b1_pos_y = m_objects[i]->m_position.y;

        int col_i = (int)(b1_pos_x / GRID_CELL_SIZE);
        int row_i = (int)(b1_pos_y / GRID_CELL_SIZE);

        const int dxdy[][2] = {
            {-1, -1}, {0, -1}, {1, -1},
            {-1, 0}, {0, 0}, {1, 0},
            {-1, 1}, {0, 1}, {1, 1}
        };

        for (int d = 0; d < 9; d++) {
            int neighbor_col = col_i + dxdy[d][0];
            int neighbor_row = row_i + dxdy[d][1];

            if (neighbor_col >= 0 && neighbor_col < m_gridColumns && neighbor_row >= 0 && neighbor_row < m_gridRows) {

                int neighbor_cell_index = neighbor_row * m_gridColumns + neighbor_col;

                for (int j : m_grid[neighbor_cell_index]) {
                    if (j <= i) {
                        continue;
                    }

                    float b2_pos_x = m_objects[j]->m_position.x;
                    float b2_pos_y = m_objects[j]->m_position.y;

                    if (circles_overlap(b1_pos_x, b1_pos_y, b2_pos_x, b2_pos_y)) {
                        float distance = sqrtf((b1_pos_x - b2_pos_x) * (b1_pos_x - b2_pos_x) + (b1_pos_y - b2_pos_y) * (b1_pos_y - b2_pos_y));
                        float overlap = 0.5f * (m_floatingObject_radius + m_floatingObject_radius - distance);

                        float nx = (b2_pos_x - b1_pos_x) / distance;
                        float ny = (b2_pos_y - b1_pos_y) / distance;

                        m_objects[i]->m_position.x -= overlap * nx;
                        m_objects[i]->m_position.y -= overlap * ny;
                        m_objects[j]->m_position.x += overlap * nx;
                        m_objects[j]->m_position.y += overlap * ny;

                        float tx = -ny;
                        float ty = nx;

                        float dptan1 = m_objects[i]->m_velocity.x * tx + m_objects[i]->m_velocity.y * ty;
                        float dptan2 = m_objects[j]->m_velocity.x * tx + m_objects[j]->m_velocity.y * ty;

                        float dpnorm1 = m_objects[i]->m_velocity.x * nx + m_objects[i]->m_velocity.y * ny;
                        float dpnorm2 = m_objects[j]->m_velocity.x * nx + m_objects[j]->m_velocity.y * ny;

                        float m1 = dpnorm2;
                        float m2 = dpnorm1;

                        m_objects[i]->m_velocity.x = tx * dptan1 + nx * m1;
                        m_objects[i]->m_velocity.y = ty * dptan1 + ny * m1;
                        m_objects[j]->m_velocity.x = tx * dptan2 + nx * m2;
                        m_objects[j]->m_velocity.y = ty * dptan2 + ny * m2;
                    }
                }
            }
        }
    }
}


void Game::dod_check_naive_collisions() {
    const int n = m_floatingObjects_id.size();

    for (int i = 0; i < n; i++) {
        float b1_pos_x = m_floatingObjects_position_x[i];
        float b1_pos_y = m_floatingObjects_position_y[i];


        for (int j = i + 1; j < n; j++) {
            float b2_pos_x = m_floatingObjects_position_x[j];
            float b2_pos_y = m_floatingObjects_position_y[j];

            if (circles_overlap(b1_pos_x, b1_pos_y, b2_pos_x, b2_pos_y)) {
                float distance = sqrtf((b1_pos_x - b2_pos_x) * (b1_pos_x - b2_pos_x) + (b1_pos_y - b2_pos_y) * (b1_pos_y - b2_pos_y));
                float overlap = 0.5f * (m_floatingObject_radius + m_floatingObject_radius - distance);

                float nx = (b2_pos_x - b1_pos_x) / distance;
                float ny = (b2_pos_y - b1_pos_y) / distance;

                m_floatingObjects_position_x[i] -= overlap * nx;
                m_floatingObjects_position_y[i] -= overlap * ny;
                m_floatingObjects_position_x[j] += overlap * nx;
                m_floatingObjects_position_y[j] += overlap * ny;

                float tx = -ny;
                float ty = nx;

                float dptan1 = m_floatingObjects_velocity_x[i] * tx + m_floatingObjects_velocity_y[i] * ty;
                float dptan2 = m_floatingObjects_velocity_x[j] * tx + m_floatingObjects_velocity_y[j] * ty;

                float dpnorm1 = m_floatingObjects_velocity_x[i] * nx + m_floatingObjects_velocity_y[i] * ny;
                float dpnorm2 = m_floatingObjects_velocity_x[j] * nx + m_floatingObjects_velocity_y[j] * ny;

                float m1 = dpnorm2;
                float m2 = dpnorm1;

                m_floatingObjects_velocity_x[i] = tx * dptan1 + nx * m1;
                m_floatingObjects_velocity_y[i] = ty * dptan1 + ny * m1;
                m_floatingObjects_velocity_x[j] = tx * dptan2 + nx * m2;
                m_floatingObjects_velocity_y[j] = ty * dptan2 + ny * m2;
            }
        }
    }
}


// pt oop
void Game::check_naive_collisions() {

    if (m_use_dod) {
        dod_check_naive_collisions();
        return;
    }

    const int n = m_floatingObjects_id.size();

    for (int i = 0; i < n; i++) {
        float b1_pos_x = m_objects[i]->m_position.x;
        float b1_pos_y = m_objects[i]->m_position.y;

        for (int j = i + 1; j < n; j++) {
            float b2_pos_x = m_objects[j]->m_position.x;
            float b2_pos_y = m_objects[j]->m_position.y;

            if (circles_overlap(b1_pos_x, b1_pos_y, b2_pos_x, b2_pos_y)) {
                float distance = sqrtf((b1_pos_x - b2_pos_x) * (b1_pos_x - b2_pos_x) + (b1_pos_y - b2_pos_y) * (b1_pos_y - b2_pos_y));
                float overlap = 0.5f * (m_floatingObject_radius + m_floatingObject_radius - distance);

                float nx = (b2_pos_x - b1_pos_x) / distance;
                float ny = (b2_pos_y - b1_pos_y) / distance;

                m_objects[i]->m_position.x -= overlap * nx;
                m_objects[i]->m_position.y -= overlap * ny;
                m_objects[j]->m_position.x += overlap * nx;
                m_objects[j]->m_position.y += overlap * ny;

                float tx = -ny;
                float ty = nx;

                float dptan1 = m_objects[i]->m_velocity.x * tx + m_objects[i]->m_velocity.y * ty;
                float dptan2 = m_objects[j]->m_velocity.x * tx + m_objects[j]->m_velocity.y * ty;

                float dpnorm1 = m_objects[i]->m_velocity.x * nx + m_objects[i]->m_velocity.y * ny;
                float dpnorm2 = m_objects[j]->m_velocity.x * nx + m_objects[j]->m_velocity.y * ny;

                float m1 = dpnorm2;
                float m2 = dpnorm1;

                m_objects[i]->m_velocity.x = tx * dptan1 + nx * m1;
                m_objects[i]->m_velocity.y = ty * dptan1 + ny * m1;
                m_objects[j]->m_velocity.x = tx * dptan2 + nx * m2;
                m_objects[j]->m_velocity.y = ty * dptan2 + ny * m2;
            }
        }
    }
}




void Game::start_counter() {
    start = SDL_GetPerformanceCounter();
}

float Game::end_counter() {
    Uint64 end = SDL_GetPerformanceCounter();
    Uint64 elapsed = end - start;

    float count_per_second = (float) SDL_GetPerformanceFrequency();
    return (elapsed * 1000.0f) / count_per_second; // ms
}



void Game::run_loop() {
    while (m_isRunning) {
        Uint64 frameStart = SDL_GetPerformanceCounter();

        start_counter();
        process_input();
        process_input_elapsed_time = end_counter();

        start_counter();
        update(m_lastDelta);
        update_elapsed_time = end_counter();

        start_counter();
        process_output();
        process_output_elapsed_time = end_counter();

        Uint64 frameEnd = SDL_GetPerformanceCounter();
        double deltaTime = (double)(frameEnd - frameStart) / (double) SDL_GetPerformanceFrequency(); // calculez elapsed time (cat a durat un frame)
        m_fps = 1.0f / deltaTime; // calculez fps-ul
        m_lastDelta = (float) deltaTime;
    }

    // opresc imgui
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    // eliberez resursele sdl
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

// Functie care verifica coliziunea intre doua cercuri (DOD)
bool Game::circles_overlap(float b1_x, float b1_y, float b2_x, float b2_y) const {
    // Formula care determina distanta dintre doua puncte (a,b) = sqrt((ax-bx)^2 + (ay-by)^2)
    float dx = (b1_x - b2_x) * (b1_x - b2_x); // calculez patratul distantei pe axa x
    float dy = (b1_y - b2_y) * (b1_y - b2_y); // calculez patratul distantei pe axa y
    float sum_radius = m_floatingObject_radius + m_floatingObject_radius; // suma razelor cercurilor

    // Doua cercuri se intersecteaza daca distanta dintre ele este mai mica sau egala cu patratul sumei razelor
    return dx + dy <= sum_radius * sum_radius;
}

// state = 1 -> to dod
// state = 0 -> to oop
void Game::sync_state(bool state) {
    if (state == 1) {
        for (int i = 0; i < m_objects.size(); i++) {
            m_floatingObjects_position_x[i] = m_objects[i]->m_position.x;
            m_floatingObjects_position_y[i] = m_objects[i]->m_position.y;
            m_floatingObjects_velocity_x[i] = m_objects[i]->m_velocity.x;
            m_floatingObjects_velocity_y[i] = m_objects[i]->m_velocity.y;
        }
    }
    else {
        for (int i = 0; i < m_objects.size(); i++) {
            m_objects[i]->m_position.x = m_floatingObjects_position_x[i];
            m_objects[i]->m_position.y = m_floatingObjects_position_y[i];
            m_objects[i]->m_velocity.x = m_floatingObjects_velocity_x[i];
            m_objects[i]->m_velocity.y = m_floatingObjects_velocity_y[i];
        }
    }
}


int Game::get_free_id() {
    return ++m_lastUsedId;
}



void Game::add_objects(int quantity) {
    for (int i = 0; i < quantity; i++) {
        int assigned_id = get_free_id();

        FloatingObject* obj = new FloatingObject(texture, renderer, assigned_id);
        m_objects.push_back(obj);

        m_floatingObjects_id.push_back(assigned_id);

        m_floatingObject_direction.push_back(obj->m_direction);

        m_floatingObjects_position_x.push_back(obj->m_position.x);
        m_floatingObjects_position_y.push_back(obj->m_position.y);

        m_floatingObjects_velocity_x.push_back(obj->m_velocity.x);
        m_floatingObjects_velocity_y.push_back(obj->m_velocity.y);
    }
}

void Game::remove_objects(int quantity) {
    for (int i = 0; i < quantity && !m_objects.empty(); i++) {
        delete m_objects.back();
        m_objects.pop_back();

        m_floatingObjects_id.pop_back();

        m_floatingObjects_position_x.pop_back();
        m_floatingObjects_position_y.pop_back();

        m_floatingObjects_velocity_x.pop_back();
        m_floatingObjects_velocity_y.pop_back();

        m_floatingObject_direction.pop_back();
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

void Game::compute_memory_usage() {
    dod_total_bytes = 0;
    oop_total_bytes = 0;
    grid_total_bytes = 0;


    dod_total_bytes += m_floatingObjects_id.capacity() * sizeof(int);
    dod_total_bytes += m_floatingObjects_position_x.capacity() * sizeof(float);
    dod_total_bytes += m_floatingObjects_position_y.capacity() * sizeof(float);
    dod_total_bytes += m_floatingObjects_velocity_x.capacity() * sizeof(float);
    dod_total_bytes += m_floatingObjects_velocity_y.capacity() * sizeof(float);
    dod_total_bytes += m_floatingObject_direction.capacity() * sizeof(float);

    oop_total_bytes += m_objects.capacity() * sizeof(FloatingObject*);
    oop_total_bytes += m_objects.size() * sizeof(FloatingObject);

    for (std::vector<int>& cell: m_grid) {
        grid_total_bytes += cell.capacity() * sizeof(int);
    }
    grid_total_bytes += m_grid.capacity() * sizeof(std::vector<int>);
}


void Game::update(float deltaTime) {
    m_floatingObject_radius = Utils::g_BALL_DIAMETER / 2.0f;

    for (int i = 0; i < m_objects.size(); i++) {
        m_objects[i]->m_radius = m_floatingObject_radius;
    }

    compute_memory_usage();

    if (m_use_dod) {
        for (int i = 0; i < m_objects.size(); i++) {
            // actualizez pozitia in functie de viteza, deltaTime si speed_multiplier (cat de repede se misca simularea)
            m_floatingObjects_position_x[i] += m_floatingObjects_velocity_x[i] * deltaTime * FloatingObject::speed_multiplier;
            m_floatingObjects_position_y[i] += m_floatingObjects_velocity_y[i] * deltaTime * FloatingObject::speed_multiplier;

            // verific coliziunea cu marginile ferestrei
            if (m_floatingObjects_position_x[i] - m_floatingObject_radius < 0.0f) { // coliziune cu marginea stanga
                m_floatingObjects_position_x[i] = m_floatingObject_radius; // repositionez obiectul
                m_floatingObjects_velocity_x[i] *= -1.0f; // mingea va merge in directia opusa
            }
            else if (m_floatingObjects_position_x[i] + m_floatingObject_radius > (float) Utils::g_WINDOW_WIDTH) { // coliziune cu marginea dreapta
                m_floatingObjects_position_x[i] = (float) Utils::g_WINDOW_WIDTH - m_floatingObject_radius; // repositionez obiectul
                m_floatingObjects_velocity_x[i] *= -1.0f;
            }

            if (m_floatingObjects_position_y[i] - m_floatingObject_radius < 0.0f) { // coliziune cu marginea de sus
                m_floatingObjects_position_y[i] = m_floatingObject_radius;
                m_floatingObjects_velocity_y[i] *= -1.0f;
            }
            else if (m_floatingObjects_position_y[i] + m_floatingObject_radius > (float) Utils::g_WINDOW_HEIGHT) { // coliziune cu marginea de jos
                m_floatingObjects_position_y[i] = (float) Utils::g_WINDOW_HEIGHT - m_floatingObject_radius;
                m_floatingObjects_velocity_y[i] *= -1.0f;
            }
        }

    }
    else {
        for (int i = 0; i < m_objects.size(); i++) {
            m_objects[i]->update(deltaTime);
        }
    }

    if (m_apply_collisions) {
        if (m_optimizedCollisions) {
            check_collisions_grid();
        }
        else {
            check_naive_collisions();
        }
    }

    // verifica daca trebuie sa adaugi sau sa stergi mingi de pe ecran
    if (m_spawn_quantity > m_objects.size()) {
        add_objects(m_spawn_quantity - m_objects.size());
    }
    else if (m_spawn_quantity < m_objects.size()) {
        remove_objects(m_objects.size() - m_spawn_quantity);
    }
    ///////

    if (m_previous_state != m_use_dod) { // daca am schimbat intre oop sau dod trebuie sa sincronizez mingile
        sync_state(m_use_dod);
        m_previous_state = m_use_dod;
    }
}

void Game::process_imgui_output() {

    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Panel");
    ImGui::Text("FPS: %.1f", m_fps);
    ImGui::Text("Update elapsed time: %.3f", update_elapsed_time);
    ImGui::Text("Process input elapsed time: %.3f", process_input_elapsed_time);
    ImGui::Text("Process output elapsed time: %.3f", process_output_elapsed_time);
    ImGui::Text("Objects: %zu", m_objects.size());
    ImGui::Text("OOP Memory usage: %.3f KB", oop_total_bytes / 1024.0f);
    ImGui::Text("DOD Memory usage: %.3f KB", dod_total_bytes / 1024.0f);
    ImGui::Text("Grid Memory usage: %.3f KB", grid_total_bytes / 1024.0f);
    ImGui::Separator();
    ImGui::SliderFloat("Speed Multiplier", &FloatingObject::speed_multiplier, 0.1f, 50.0f);
    ImGui::Separator();
    ImGui::SliderInt("Spawned objects", &m_spawn_quantity, 0, MAX_ENTITIES);
    ImGui::Separator();
    ImGui::SliderInt("Balls diameter", &Utils::g_BALL_DIAMETER, 1, 50);

    ImGui::Separator();
    ImGui::Text("Render Mode:");
    ImGui::RadioButton("OOP", &m_use_dod, 0);
    ImGui::SameLine();
    ImGui::RadioButton("DOD", &m_use_dod, 1);

    ImGui::Separator();
    ImGui::Checkbox("Collisions", &m_apply_collisions);

    ImGui::Separator();
    ImGui::Checkbox("Draw grid", &m_drawGrid);

    ImGui::Separator();
    ImGui::Checkbox("Optimized collisions", &m_optimizedCollisions);

    ImGui::Separator();
    ImGui::Text("Grid cell size");

    std::vector<int> divisors = {200, 50, 40, 25, 20, 10, 8, 5, 4, 2, 1};

    if (GRID_CELL_SIZE <= Utils::g_BALL_DIAMETER) {
        for (int i = divisors.size() - 1; i >= 0; i--) {
            if (divisors[i] > Utils::g_BALL_DIAMETER) {
                GRID_CELL_SIZE = divisors[i];
                break;
            }
        }
    }

    for (int divisor: divisors) {
        if (divisor > Utils::g_BALL_DIAMETER) {
            std::string d = std::to_string(divisor);
            ImGui::RadioButton(d.c_str(), &GRID_CELL_SIZE, divisor);
            ImGui::SameLine();
        }
    }

    ImGui::End();
}


void Game::process_output() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);

    process_imgui_output();

    if (m_drawGrid) {
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

        // Ca sa desenez fiecare celula prima data trasez linii verticale si dupa linii orizontale
        // linii verticale

        for (int i = 0; i <= m_gridColumns; i++) {
            float x = (float)(i * GRID_CELL_SIZE);
            SDL_RenderLine(renderer, x, 0.0f, x, (float)Utils::g_WINDOW_HEIGHT);
        }

        // liniile orizontale
        for (int i = 0; i <= m_gridRows; i++) {
            float y = (float)(i * GRID_CELL_SIZE);
            SDL_RenderLine(renderer, 0.0f, y, (float)Utils::g_WINDOW_WIDTH, y);
        }
    }

    // prima data dau render la obiecte
    if (m_use_dod) {
        for (int i = 0; i < m_objects.size(); i++) {
            SDL_FRect destRect = {
                m_floatingObjects_position_x[i] - m_floatingObject_radius, // scad radius pentru ca originea este in centru
                m_floatingObjects_position_y[i] - m_floatingObject_radius, // scad radius pentru ca originea este in centru
                (float) Utils::g_BALL_DIAMETER,
                (float) Utils::g_BALL_DIAMETER
            };
            SDL_RenderTexture(renderer, texture, NULL, &destRect);
        }
    }
    else {
        for (int i = 0; i < m_objects.size(); i++) {
            m_objects[i]->render();
        }
    }

    // vreau ca fereastra pentru imgui sa fie mereu deasupra
    ImGui::Render();
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);

    SDL_RenderPresent(renderer);
}