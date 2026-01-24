#include "Game.h"
#include <random>
#include <ctime>
#include "constants.h"
#include "imgui.h"

int Constants::g_WINDOW_WIDTH = 1600;
int Constants::g_WINDOW_HEIGHT = 1000;
int Constants::g_BALL_DIAMETER = 4;
float Constants::g_SIMULATION_SPEED = 1.0f;

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