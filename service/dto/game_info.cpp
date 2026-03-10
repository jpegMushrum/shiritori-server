#include <iostream>

#include "game_info.hpp"

using ull = unsigned long long;

GameInfo::GameInfo() : id(0), wordsCount(0), playersCount(0) {}

GameInfo::GameInfo(ull id, int wordsCount, int playersCount)
    : id(id), wordsCount(wordsCount), playersCount(playersCount)
{
}

std::ostream& operator<<(std::ostream& out, const GameInfo& gi)
{
    out << gi.id << " " << gi.wordsCount << " " << gi.playersCount;
    return out;
}