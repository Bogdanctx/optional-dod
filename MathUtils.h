#include <cmath>
#include <SDL3/SDL.h>

class MathUtils {
public:
    static bool circles_overlap(float x1, float y1, float x2, float y2, float radius) 
    {
        // Formula care determina distanta dintre doua puncte (a,b) = sqrt((ax-bx)^2 + (ay-by)^2)
        float dx = (x1 - x2) * (x1 - x2); // calculez patratul distantei pe axa x
        float dy = (y1 - y2) * (y1 - y2); // calculez patratul distantei pe axa y
        float sum_radius = radius + radius; // suma razelor cercurilor

        // Doua cercuri se intersecteaza daca distanta dintre ele este mai mica sau egala cu patratul sumei razelor
        return dx + dy <= sum_radius * sum_radius;
    }

    static void resolve_elastic_collision(float& x1, float& y1, float& vx1, float& vy1,
                                          float& x2, float& y2, float& vx2, float& vy2,
                                          float radius) 
    {
        float distance = sqrtf((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
        if (distance < 0.0001f) {
            distance = 0.0001f;
        }

        float overlap = 0.5f * (radius + radius - distance);

        float nx = (x2 - x1) / distance;
        float ny = (y2 - y1) / distance;

        x1 -= overlap * nx;
        y1 -= overlap * ny;
        x2 += overlap * nx;
        y2 += overlap * ny;

        float kx = (vx1 - vx2);
        float ky = (vy1 - vy2);
        float p = 2.0f * (nx * kx + ny * ky) / 2.0f; // toate mingile au masa=1

        vx1 = vx1 - p * nx;
        vy1 = vy1 - p * ny;
        vx2 = vx2 + p * nx;
        vy2 = vy2 + p * ny;
    }
};