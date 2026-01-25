#ifndef DOD_CHRONOMETER_H
#define DOD_CHRONOMETER_H

#include <SDL3/SDL.h>

class Chronometer {
public:
    static void start();
    static Uint64 stop();
private:
    static Uint64 m_startTime;
};

#endif //DOD_CHRONOMETER_H