#include "FloatingObject.h"
#include "constants.h"

float FloatingObject::speed_multiplier = 1.0f;

FloatingObject::FloatingObject(SDL_Texture* texture, SDL_Renderer *renderer, int id) 
    : m_texture(texture), m_renderer(renderer), m_id(id)
{
    m_radius = Constants::g_BALL_DIAMETER / 2.0f;
    m_direction = rand() % 360;
    
    float safeX = rand() % (Constants::g_WINDOW_WIDTH - (int)Constants::g_BALL_DIAMETER) + Constants::g_BALL_DIAMETER;
    float safeY = rand() % (Constants::g_WINDOW_HEIGHT - (int)Constants::g_BALL_DIAMETER) + Constants::g_BALL_DIAMETER;
    
    m_position = {safeX, safeY};

    float radians = m_direction * SDL_PI_F / 180.0f;
    m_velocity = {cosf(radians) * m_speed, sinf(radians) * m_speed};
}

void FloatingObject::update(float deltaTime) {
    m_velocity.y += Constants::g_GRAVITY * deltaTime;

    m_position.x += m_velocity.x * deltaTime;
    m_position.y += m_velocity.y * deltaTime;
}

void FloatingObject::render() {
    SDL_FRect destRect = {
        m_position.x - m_radius,
        m_position.y - m_radius,
        (float) Constants::g_BALL_DIAMETER,
        (float) Constants::g_BALL_DIAMETER
    };
    SDL_RenderTexture(m_renderer, m_texture, NULL, &destRect);
}