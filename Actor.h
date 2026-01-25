#pragma once
#include <SDL3/SDL.h>
#include <random>
#include <cmath>
#include "actor_type.h"

class Actor {
public:
    Actor(std::vector<SDL_Texture*> texture, SDL_Renderer *renderer, int id, int status);

    int m_id;
    SDL_FPoint m_position = { 0, 0 };
    SDL_FPoint m_velocity = {50.f, 50.f};
    std::vector<SDL_Texture*> m_textures;

    float m_direction = 0; 
    float m_radius = 32.0f; 
    float m_speed = 100.0f;
    int m_status;

    void update(float deltaTime);
    void render();

private:
    SDL_Renderer* m_renderer = NULL;
};