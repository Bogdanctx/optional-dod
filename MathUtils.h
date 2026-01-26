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

    // Cod preluat de la Javidx9
    static void resolve_collision(float& x1, float& y1, float& vx1, float& vy1,
                                          float& x2, float& y2, float& vx2, float& vy2,
                                          float radius)
    {
        // calculez distanta dintre centrele cercurilor
        float dx = x1 - x2;
        float dy = y1 - y2;
        float distance = std::sqrt(dx*dx + dy*dy);

        // sa nu am impartire la 0
        if (distance < 0.0001f) {
            distance = 0.0001f;
        }

        float overlap = 0.5f * (radius + radius - distance);

        // separ cercurile (se misca in directia vectorului)
        float normalized_x = (x2 - x1) / distance; // normalizare pentru stabilitate
        float normalized_y = (y2 - y1) / distance; // normalizare pentru stabilitate
        x1 -= overlap * normalized_x;
        y1 -= overlap * normalized_y;
        x2 += overlap * normalized_x;
        y2 += overlap * normalized_y;

        // tangenta
        float tx = -normalized_y;
        float ty = normalized_x;

        // dot product tangent
        float dpTan1 = vx1 * tx + vy1 * ty;
        float dpTan2 = vx2 * tx + vy2 * ty;

        // dot product normal
        float dpNorm1 = vx1 * normalized_x + vy1 * normalized_y;
        float dpNorm2 = vx2 * normalized_x + vy2 * normalized_y;

        // ellastic collisions -> toate cercurile au masa 1
        float m1 = (dpNorm1 * (1 - 1) + 2.0f * 1 * dpNorm2) / (1 + 1);
        float m2 = (dpNorm2 * (1 - 1) + 2.0f * 1 * dpNorm1) / (1 + 1);

        // actualizare velocitate
        vx1 = tx * dpTan1 + normalized_x * m1;
        vy1 = ty * dpTan1 + normalized_y * m1;
        vx2 = tx * dpTan2 + normalized_x * m2;
        vy2 = ty * dpTan2 + normalized_y * m2;
    }
};