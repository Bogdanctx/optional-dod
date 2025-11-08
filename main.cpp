#include "Game.h"
#include <random>
#include <ctime>
#include "utils.h"
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

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