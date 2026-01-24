#pragma once
#include <SDL3/SDL.h>
#include <random>
#include <cmath>

class FloatingObject {
public:
    FloatingObject(SDL_Texture* texture, SDL_Renderer *renderer, int id);

    int m_id;
    static float speed_multiplier;
    SDL_FPoint m_position = { 0, 0 };
    SDL_FPoint m_velocity = {50.f, 50.f};
    
    float m_direction = 0; 
    float m_radius = 32.0f; 
    float m_speed = 100.0f; 

    void update(float deltaTime);
    void render();

private:
    SDL_Texture* m_texture = NULL;
    SDL_Renderer* m_renderer = NULL;
};