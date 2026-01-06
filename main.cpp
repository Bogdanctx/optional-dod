#include "Game.h"
#include <random>
#include <ctime>
#include "utils.h"
#include "imgui.h"

int Utils::g_WINDOW_WIDTH = 1600;
int Utils::g_WINDOW_HEIGHT = 1000;
int Utils::g_BALL_DIAMETER = 4;

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