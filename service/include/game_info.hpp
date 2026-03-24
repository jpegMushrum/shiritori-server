#pragma once

#include <iostream>

using ull = unsigned long long;

class GameInfo
{
  public:
    GameInfo();
    GameInfo(ull, ull, int, int);

    ull gameId;
    ull userId;
    int words;
    int place;

    friend std::ostream& operator<<(std::ostream&, const GameInfo&);
};