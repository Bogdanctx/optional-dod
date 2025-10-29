#include "Game.h"

int main() {
    Game game;
    bool created = game.init();
    if (!created) {
        return 1;
    }
    game.run_loop();

    return 0;
}