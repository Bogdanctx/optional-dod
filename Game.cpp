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

    SDL_Surface* surface = SDL_LoadPNG("ball.png");
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_DestroySurface(surface);

    m_isRunning = true;

    return true;
}


void Game::run_loop() {
    while (m_isRunning) {
        process_input();
        update();
        process_output();
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

bool circles_overlap(FloatingObject* a_fo, FloatingObject* b_fo) {
    SDL_FPoint b1 = a_fo->m_position;
    SDL_FPoint b2 = b_fo->m_position;

    float dx = (b1.x - b2.x) * (b1.x - b2.x);
    float dy = (b1.y - b2.y) * (b1.y - b2.y);
    float sum_radius = a_fo->get_radius() + b_fo->get_radius();

    return fabsf(dx + dy) <= sum_radius * sum_radius;
}

void Game::check_collisions() {
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

                // formula elastic collision
                float m1 = (dpnorm1 * (b1->m_mass - b2->m_mass) + 2.0f * b2->m_mass * dpnorm2) / (b1->m_mass + b2->m_mass);
                float m2 = (dpnorm2 * (b2->m_mass - b1->m_mass) + 2.0f * b1->m_mass * dpnorm1) / (b1->m_mass + b2->m_mass);

                b1->m_velocity = SDL_FPoint{tx * dptan1 + nx * m1, ty * dptan1 + ny * m1};
                b2->m_velocity = SDL_FPoint{tx * dptan2 + nx * m2, ty * dptan2 + ny * m2};
            }
        }
    }
}


void Game::add_object() {
    m_objects.push_back(
        new FloatingObject(texture, renderer, m_objects.size() + 1)
    );
}

void Game::process_input() {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_EVENT_QUIT: {
                m_isRunning = false;
                break;
            }
            case SDL_EVENT_KEY_DOWN: {
                SDL_KeyboardEvent keyboard_event = event.key;
                SDL_Keycode key_pressed = keyboard_event.key;

                switch (key_pressed) {
                    case SDLK_ESCAPE: {
                        m_isRunning = false;
                        break;
                    }
                    case SDLK_K: {
                        add_object();
                        break;
                    }
                    case SDLK_SPACE: {
                        int quantity = 0;
                        std::cout << "Remove/Spawn objects: "; std::cin >> quantity;
                        if (quantity >= 0) {
                            for (int i = 0; i < quantity; i++) {
                                add_object();
                            }
                        }
                        else {
                            for (int i = 0; i < quantity || i < m_objects.size(); i++) {
                                delete m_objects[i];
                            }
                        }
                        break;
                    }
                    case SDLK_LEFT: {
                        FloatingObject::decrease_speed();
                        break;
                    }
                    case SDLK_RIGHT: {
                        FloatingObject::increase_speed();
                        break;
                    }
                }

                break;
            }
            default:
                break;
        }
    }
}

void Game::update() {
    static Uint64 lastTime = SDL_GetTicks();
    Uint64 current = SDL_GetTicks();
    float deltaTime = (current - lastTime) / 1000.0f;
    lastTime = current;

    for (int i = 0; i < m_objects.size(); i++) {
        m_objects[i]->update(deltaTime);
    }

    check_collisions();
}

void Game::process_output() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 0, 0, 255, SDL_ALPHA_OPAQUE);
    SDL_RenderFillRect(renderer, &dummy_rect);

    for (int i = 0; i < m_objects.size(); i++) {
        m_objects[i]->render();
    }

    SDL_RenderPresent(renderer);
}