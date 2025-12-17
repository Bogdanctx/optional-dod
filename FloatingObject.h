#include <SDL3/SDL.h>
#include <random>
#include <cmath>

class FloatingObject {
private:
    SDL_Texture* m_texture = NULL;
    SDL_Renderer* m_renderer = NULL;
    float m_radius = 32.0f;
    float m_speed = 100.0f;
public:
    FloatingObject(SDL_Texture* texture, SDL_Renderer *renderer);

    static float speed_multiplier;
    SDL_FPoint m_position = { 0, 0 };
    SDL_FPoint m_velocity = {50.f, 50.f};
    float m_direction = 0;

    void update(float deltaTime);
    void render();

    float get_radius();
};