#pragma once
#include <cmath>
#include <algorithm>
#include <SDL3/SDL.h>

class MathUtils {
public:
    static bool circles_overlap(float x1, float y1, float x2, float y2, float radius)
    {
        float dx = (x1 - x2) * (x1 - x2);
        float dy = (y1 - y2) * (y1 - y2);
        float sum_radius = radius + radius;
        return dx + dy <= sum_radius * sum_radius;
    }

    static void resolve_elastic_collision(float& x1, float& y1, float& vx1, float& vy1,
                                          float& x2, float& y2, float& vx2, float& vy2,
                                          float radius)
    {
        float dx = x1 - x2;
        float dy = y1 - y2;
        float distance = std::sqrt(dx*dx + dy*dy);

        if (distance < 0.0001f) distance = 0.0001f;

        float overlap = 0.5f * (radius + radius - distance);
        float nx = (x2 - x1) / distance;
        float ny = (y2 - y1) / distance;

        x1 -= overlap * nx;
        y1 -= overlap * ny;
        x2 += overlap * nx;
        y2 += overlap * ny;

        float kx = (vx1 - vx2);
        float ky = (vy1 - vy2);
        float p = 2.0f * (nx * kx + ny * ky) / 2.0f;

        vx1 = vx1 - p * nx;
        vy1 = vy1 - p * ny;
        vx2 = vx2 + p * nx;
        vy2 = vy2 + p * ny;
    }
};