#pragma once

#include <iostream>
#include <uchar.h>

using ull = unsigned long long;

class GameContext
{
  public:
    GameContext();
    GameContext(ull, int, int, ull);

    ull id;
    int wordsCount;
    int playersCount;

    ull adminId;
    ull lastPlayerId;

    char32_t lastKana;

    friend std::ostream& operator<<(std::ostream&, const GameContext&);
};