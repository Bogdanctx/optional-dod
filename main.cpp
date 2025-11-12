#include "Game.h"
#include <random>
#include <ctime>
#include "utils.h"
#include "imgui.h"

int Utils::g_WINDOW_WIDTH = 1600;
int Utils::g_WINDOW_HEIGHT = 1000;
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