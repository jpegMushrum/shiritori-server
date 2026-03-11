#include <iostream>

#include "game_context.hpp"

using ull = unsigned long long;

GameContext::GameContext() : id(0), wordsCount(0), playersCount(0), adminId(0), lastPlayerId(0) {}

GameContext::GameContext(ull id, int wordsCount, int playersCountm, ull adminId)
    : id(id), wordsCount(wordsCount), playersCount(playersCount), adminId(adminId)
{
}

std::ostream& operator<<(std::ostream& out, const GameContext& gi)
{
    out << gi.id << " " << gi.wordsCount << " " << gi.playersCount;
    return out;
}