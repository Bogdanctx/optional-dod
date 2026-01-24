#include "FloatingObject.h"

#include <utility>
#include "constants.h"

FloatingObject::FloatingObject(std::vector<SDL_Texture*> textures, SDL_Renderer *renderer, int id, int status)
    : m_textures(std::move(textures)), m_renderer(renderer), m_id(id), m_status(status)
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
    SDL_RenderTexture(m_renderer, m_textures[m_status], NULL, &destRect);
}