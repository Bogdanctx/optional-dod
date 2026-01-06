#include "FloatingObject.h"
#include "utils.h"

float FloatingObject::speed_multiplier = 1.0f;

FloatingObject::FloatingObject(SDL_Texture* texture, SDL_Renderer *renderer, int id) {
    m_texture = texture;
    m_renderer = renderer;
    m_radius = Utils::g_BALL_DIAMETER / 2.0f;

    m_direction = rand() % 360;
    float random_x = rand() % (Utils::g_WINDOW_WIDTH - (int) Utils::g_BALL_DIAMETER) + Utils::g_BALL_DIAMETER; // preventie spawn initial langa margini
    float random_y = rand() % (Utils::g_WINDOW_HEIGHT - (int) Utils::g_BALL_DIAMETER) + Utils::g_BALL_DIAMETER; // preventie spawn initial langa margini
    m_position = SDL_FPoint{random_x + m_radius, random_y + m_radius}; // pun originea in centru

    float radians = m_direction * M_PI / 180.0f;

    m_velocity = SDL_FPoint{cosf(radians) * m_speed, sinf(radians) * m_speed};

    m_id = id;
}

void FloatingObject::update(float deltaSpeed) {
    // actualizez pozitia in functie de viteza, deltaTime si speed_multiplier (cat de repede se misca simularea)
    m_position.x += m_velocity.x * deltaSpeed * speed_multiplier;
    m_position.y += m_velocity.y * deltaSpeed * speed_multiplier;

    // verific coliziunea cu marginile ferestrei
    if (m_position.x - m_radius < 0.0f) {
        m_position.x = m_radius;
        m_velocity.x *= -1.0f;
    }
    else if (m_position.x + m_radius > (float) Utils::g_WINDOW_WIDTH) {
        m_position.x = (float) Utils::g_WINDOW_WIDTH - m_radius;
        m_velocity.x *= -1.0f;
    }

    if (m_position.y - m_radius < 0.0f) {
        m_position.y = m_radius;
        m_velocity.y *= -1.0f;
    }
    else if (m_position.y + m_radius > (float) Utils::g_WINDOW_HEIGHT) {
        m_position.y = (float) Utils::g_WINDOW_HEIGHT - m_radius;
        m_velocity.y *= -1.0f;
    }
    ///////////////////////////////////////////////
}

void FloatingObject::render() {
    SDL_FRect destRect = {
        m_position.x - m_radius,
        m_position.y - m_radius,
        Utils::g_BALL_DIAMETER,
        Utils::g_BALL_DIAMETER
    };
    SDL_RenderTexture(m_renderer, m_texture, NULL, &destRect);
}