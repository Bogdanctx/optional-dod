#include "Game.h"
#include <random>
#include "utils.h"

int Utils::g_WINDOW_WIDTH = 1200;
int Utils::g_WINDOW_HEIGHT = 800;
float Utils::g_BALL_DIAMETER = 64;

int main() {
    srand(time(nullptr));

    Game game;
    bool created = game.init();
    if (!created) {
        return 1;
    }
    game.run_loop();

    return 0;
}