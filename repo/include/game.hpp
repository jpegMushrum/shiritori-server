#pragma once

#include <vector>

using ull = unsigned long long;

class Game
{
  public:
    Game();
    Game(ull, ull, int, int);

    ull gameId;
    ull userId;
    int words;
    int place;
};