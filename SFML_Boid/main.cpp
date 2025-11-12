#include <stdio.h>
#include "Game.h"

int main() 
{
    Game game;
    srand(static_cast<unsigned>(time(nullptr)));
    game.Run();
    return 0;
}