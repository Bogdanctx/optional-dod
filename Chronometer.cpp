#include "Chronometer.h"

Uint64 Chronometer::m_startTime = 0;

void Chronometer::start() {
    m_startTime = SDL_GetPerformanceCounter();
}

Uint64 Chronometer::stop() {
    Uint64 end = SDL_GetPerformanceCounter();

    return end - m_startTime;
}