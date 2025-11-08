#include "Game.h"
#include "utils.h"
#include <SDL3/SDL_keycode.h>
#include <iostream>

Game::~Game() {
    for (int i = 0; i < m_objects.size(); i++) {
        delete m_objects[i];
    }
}

bool Game::init() {
    SDL_Init(SDL_INIT_VIDEO);

    window = SDL_CreateWindow("Proiect DOD", Utils::g_WINDOW_WIDTH, Utils::g_WINDOW_HEIGHT, SDL_WINDOW_OPENGL);

    if (window == NULL) {
        return false;
    }

    renderer = SDL_CreateRenderer(window, NULL);

    if (renderer == NULL) {
        return false;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer3_Init(renderer);

    SDL_Surface* surface = IMG_Load("ball.png");
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_DestroySurface(surface);

    m_isRunning = true;

    return true;
}

void Game::run_loop() {
    while (m_isRunning) {
        Uint64 frameStart = SDL_GetPerformanceCounter();

        process_input();
        update(m_lastDelta);
        process_output();

        Uint64 frameEnd = SDL_GetPerformanceCounter();
        double deltaTime = (double)(frameEnd - frameStart) / SDL_GetPerformanceFrequency();
        m_fps = 1.0f / deltaTime;
        m_lastDelta = (float) deltaTime;
    }

    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

bool Game::circles_overlap(int id1, int id2) {
    SDL_FPoint b1 = m_floatingObject_position[id1];
    SDL_FPoint b2 = m_floatingObject_position[id2];

    float dx = (b1.x - b2.x) * (b1.x - b2.x);
    float dy = (b1.y - b2.y) * (b1.y - b2.y);
    float sum_radius = m_floatingObject_radius + m_floatingObject_radius;

    return fabsf(dx + dy) <= sum_radius * sum_radius;
}

bool Game::circles_overlap(FloatingObject* a_fo, FloatingObject* b_fo) {
    SDL_FPoint b1 = a_fo->m_position;
    SDL_FPoint b2 = b_fo->m_position;

    float dx = (b1.x - b2.x) * (b1.x - b2.x);
    float dy = (b1.y - b2.y) * (b1.y - b2.y);
    float sum_radius = a_fo->get_radius() + b_fo->get_radius();

    return fabsf(dx + dy) <= sum_radius * sum_radius;
}

void Game::dod_check_collisions() {
    for (int i = 0; i < m_floatingObject_position.size(); i++) {
        for (int j = i + 1; j < m_floatingObject_position.size(); j++) {
            if (circles_overlap(i, j)) {
                SDL_FPoint b1_pos = m_floatingObject_position[i];
                SDL_FPoint b2_pos = m_floatingObject_position[j];

                float distance = sqrtf((b1_pos.x - b2_pos.x) * (b1_pos.x - b2_pos.x) + (b1_pos.y - b2_pos.y) * (b1_pos.y - b2_pos.y));
                if (distance < 0.00001f) { // daca se spawneaza mingile una peste alta o dau pe una intr-o parte
                    m_floatingObject_position[i].x += 0.1f;
                    b1_pos = m_floatingObject_position[i];
                    distance = sqrtf((b1_pos.x - b2_pos.x) * (b1_pos.x - b2_pos.x) + (b1_pos.y - b2_pos.y) * (b1_pos.y - b2_pos.y));
                }

                float overlap = 0.5f * (m_floatingObject_radius + m_floatingObject_radius - distance);

                // normala
                float nx = (b2_pos.x - b1_pos.x) / distance;
                float ny = (b2_pos.y - b1_pos.y) / distance;

                // displace-uri
                m_floatingObject_position[i].x -= overlap * nx;
                m_floatingObject_position[i].y -= overlap * ny;
                m_floatingObject_position[j].x += overlap * nx;
                m_floatingObject_position[j].y += overlap * ny;

                // tangenta
                float tx = -ny;
                float ty = nx;

                // produs scalar intre tangente
                float dptan1 = m_floatingObject_velocity[i].x * tx + m_floatingObject_velocity[i].y * ty;
                float dptan2 = m_floatingObject_velocity[j].x * tx + m_floatingObject_velocity[j].y * ty;

                // produs scalar intre normale
                float dpnorm1 = m_floatingObject_velocity[i].x * nx + m_floatingObject_velocity[i].y * ny;
                float dpnorm2 = m_floatingObject_velocity[j].x * nx + m_floatingObject_velocity[j].y * ny;

                // formula elastic collision (pentru simplificare mass = 1)
                float m1 = dpnorm2;
                float m2 = dpnorm1;

                m_floatingObject_velocity[i] = SDL_FPoint{tx * dptan1 + nx * m1, ty * dptan1 + ny * m1};
                m_floatingObject_velocity[j] = SDL_FPoint{tx * dptan2 + nx * m2, ty * dptan2 + ny * m2};
            }
        }
    }
}


void Game::check_collisions() {
    if (m_use_dod) {
        dod_check_collisions();
        return;
    }

    for (int i = 0; i < m_objects.size(); i++) {
        FloatingObject* b1 = m_objects[i];
        for (int j = i + 1; j < m_objects.size(); j++) {
            FloatingObject* b2 = m_objects[j];

            if (circles_overlap(b1, b2)) {
                SDL_FPoint b1_pos = b1->m_position;
                SDL_FPoint b2_pos = b2->m_position;

                float distance = sqrtf((b1_pos.x - b2_pos.x) * (b1_pos.x - b2_pos.x) + (b1_pos.y - b2_pos.y) * (b1_pos.y - b2_pos.y));
                if (distance < 0.00001f) { // daca se spawneaza mingile una peste alta o dau pe una intr-o parte
                    b1->m_position.x += 0.1f;
                    b1_pos = b1->m_position;
                    distance = sqrtf((b1_pos.x - b2_pos.x) * (b1_pos.x - b2_pos.x) + (b1_pos.y - b2_pos.y) * (b1_pos.y - b2_pos.y));
                }

                float overlap = 0.5f * (b1->get_radius() + b2->get_radius() - distance);

                // normala
                float nx = (b2_pos.x - b1_pos.x) / distance;
                float ny = (b2_pos.y - b1_pos.y) / distance;

                // displace-uri
                b1->m_position.x -= overlap * nx;
                b1->m_position.y -= overlap * ny;
                b2->m_position.x += overlap * nx;
                b2->m_position.y += overlap * ny;

                // tangenta
                float tx = -ny;
                float ty = nx;

                // produs scalar intre tangente
                float dptan1 = b1->m_velocity.x * tx + b1->m_velocity.y * ty;
                float dptan2 = b2->m_velocity.x * tx + b2->m_velocity.y * ty;

                // produs scalar intre normale
                float dpnorm1 = b1->m_velocity.x * nx + b1->m_velocity.y * ny;
                float dpnorm2 = b2->m_velocity.x * nx + b2->m_velocity.y * ny;

                // formula elastic collision (mass = 1 pentru simplificare)
                float m1 = dpnorm2;
                float m2 = dpnorm1;

                b1->m_velocity = SDL_FPoint{tx * dptan1 + nx * m1, ty * dptan1 + ny * m1};
                b2->m_velocity = SDL_FPoint{tx * dptan2 + nx * m2, ty * dptan2 + ny * m2};
            }
        }
    }
}


void Game::add_objects(int quantity) {
    for (int i = 0; i < quantity; i++) {
        FloatingObject* obj = new FloatingObject(texture, renderer, m_objects.size() + 1);
        m_objects.push_back(obj);

        m_floatingObject_direction.push_back(obj->m_direction);
        m_floatingObject_position.push_back(obj->m_position);
        m_floatingObject_velocity.push_back(obj->m_velocity);
        m_floatingObject_id.push_back(obj->m_id);
    }
}

void Game::remove_objects(int quantity) {
    for (int i = 0; i < quantity && !m_objects.empty(); i++) {
        delete m_objects.back();
        m_objects.pop_back();

        m_floatingObject_id.pop_back();
        m_floatingObject_position.pop_back();
        m_floatingObject_velocity.pop_back();
        m_floatingObject_direction.pop_back();
    }
}

void Game::m_floatingObject_update(float deltaSpeed, int i) {
    m_floatingObject_position[i].x += m_floatingObject_velocity[i].x * deltaSpeed * FloatingObject::speed_multiplier;
    m_floatingObject_position[i].y += m_floatingObject_velocity[i].y * deltaSpeed * FloatingObject::speed_multiplier;

    if (m_floatingObject_position[i].x - m_floatingObject_radius < 0.0f) {
        m_floatingObject_position[i].x = m_floatingObject_radius;
        m_floatingObject_velocity[i].x *= -1.0f;
    }
    else if (m_floatingObject_position[i].x + m_floatingObject_radius > (float) Utils::g_WINDOW_WIDTH) {
        m_floatingObject_position[i].x = (float) Utils::g_WINDOW_WIDTH - m_floatingObject_radius;
        m_floatingObject_velocity[i].x *= -1.0f;
    }

    if (m_floatingObject_position[i].y - m_floatingObject_radius < 0.0f) {
        m_floatingObject_position[i].y = m_floatingObject_radius;
        m_floatingObject_velocity[i].y *= -1.0f;
    }
    else if (m_floatingObject_position[i].y + m_floatingObject_radius > (float) Utils::g_WINDOW_HEIGHT) {
        m_floatingObject_position[i].y = (float) Utils::g_WINDOW_HEIGHT - m_floatingObject_radius;
        m_floatingObject_velocity[i].y *= -1.0f;
    }
}

void Game::process_input() {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        ImGui_ImplSDL3_ProcessEvent(&event);
        ImGuiIO& io = ImGui::GetIO();
        bool capture_keyboard = io.WantCaptureKeyboard;

        switch (event.type) {
            case SDL_EVENT_QUIT: {
                m_isRunning = false;
                break;
            }
            case SDL_EVENT_KEY_DOWN: {
                if (!capture_keyboard) {
                    SDL_KeyboardEvent keyboard_event = event.key;
                    SDL_Keycode key_pressed = keyboard_event.key;

                    switch (key_pressed) {
                        case SDLK_ESCAPE: {
                            m_isRunning = false;
                            break;
                        }
                        case SDLK_K: {
                            add_objects(1);
                            break;
                        }
                    }
                }

                break;
            }
            default:
                break;
        }
    }
}

void Game::update(float deltaTime) {
    if (m_use_dod) {
        for (int i = 0; i < m_objects.size(); i++) {
            m_floatingObject_update(deltaTime, i);
        }
    }
    else {
        for (int i = 0; i < m_objects.size(); i++) {
            m_objects[i]->update(deltaTime);
        }
    }

    check_collisions();
}

void Game::m_floatingObject_render(int i) {
    SDL_FRect destRect = {
        m_floatingObject_position[i].x - m_floatingObject_radius,
        m_floatingObject_position[i].y - m_floatingObject_radius,
        Utils::g_BALL_DIAMETER,
        Utils::g_BALL_DIAMETER
    };
    SDL_RenderTexture(renderer, texture, NULL, &destRect);
}


void Game::process_output() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);

    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    {
        ImGui::Begin("Panel");
        ImGui::Text("FPS: %.1f", m_fps);
        ImGui::Text("Objects: %zu", m_objects.size());
        ImGui::Separator();
        ImGui::SliderFloat("Speed Multiplier", &FloatingObject::speed_multiplier, 0.1f, 50.0f);
        ImGui::Separator();
        ImGui::SliderInt("Spawned objects", &m_spawn_quantity, 0, 100000);

        if (m_spawn_quantity > m_objects.size()) {
            add_objects(m_spawn_quantity - m_objects.size());
        }
        else if (m_spawn_quantity < m_objects.size()) {
            remove_objects(m_objects.size() - m_spawn_quantity);
        }

        ImGui::Separator();
        ImGui::Text("Render Mode:");
        ImGui::RadioButton("OOP", &m_use_dod, 0); // Sets m_use_dod to false
        ImGui::SameLine();
        ImGui::RadioButton("DOD", &m_use_dod, 1);  // Sets m_use_dod to true
        ImGui::End();
    }

    // prima data dau render la obiecte

    if (m_use_dod) {
        for (int i = 0; i < m_objects.size(); i++) {
            m_floatingObject_render(i);
        }
    }
    else {
        for (int i = 0; i < m_objects.size(); i++) {
            m_objects[i]->render();
        }
    }

    // vreau ca dreptunghiul sa fie mereu deasupra
    SDL_SetRenderDrawColor(renderer, 0, 0, 255, SDL_ALPHA_OPAQUE);
    SDL_RenderFillRect(renderer, &dummy_rect);

    ImGui::Render();
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);

    SDL_RenderPresent(renderer);
}