#include "game_info.hpp"

GameInfo::GameInfo() : gameId(0), userId(0), words(0), place(0) {}

GameInfo::GameInfo(ull gameId, ull userId, int words, int place)
    : gameId(gameId), userId(userId), words(words), place(place)
{
}

std::ostream& operator<<(std::ostream& out, const GameInfo& gi)
{
    out << gi.gameId << " " << gi.userId << " " << gi.words << " " << gi.place;
    return out;
}