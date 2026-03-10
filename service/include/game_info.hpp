#pragma once

#include <iostream>

using ull = unsigned long long;

class GameInfo
{
  public:
    GameInfo();
    GameInfo(ull, int, int);

    ull id;
    int wordsCount;
    int playersCount;

    friend std::ostream& operator<<(std::ostream&, const GameInfo&);
};