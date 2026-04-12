#include "game_context.hpp"

using ull = unsigned long long;

GameContext::GameContext() : id(0), wordsCount(0), playersCount(0), adminId(0), lastPlayerId(0) {}

GameContext::GameContext(ull id, int wordsCount, int playersCount, ull adminId, char32_t lastKana)
    : id(id), wordsCount(wordsCount), playersCount(playersCount), adminId(adminId), lastPlayerId(0),
      lastKana(lastKana)
{
}

std::ostream& operator<<(std::ostream& out, const GameContext& gc)
{
    std::u32string u32kana(1, gc.lastKana);
    std::string lastKana = boost::locale::conv::utf_to_utf<char>(u32kana);

    out << gc.id << " " << gc.wordsCount << " " << gc.playersCount << " " << gc.adminId << " "
        << gc.lastPlayerId << " " << lastKana;
    return out;
}