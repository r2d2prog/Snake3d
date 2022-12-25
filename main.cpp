#include <iostream>
#include "game.h"

int main()
{
    Game* game = new Game();
    game->Update();
    delete game;
    return 0;
}
