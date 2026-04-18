#pragma once

#include <optional>
#include <vector>

#include "player_score.hpp"
#include "word_info.hpp"

struct GameUpdateEvent
{
    enum Type
    {
        WORD_PLAYED,
        GAME_STOPPED
    } type;

    // For WORD_PLAYED
    std::optional<WordInfo> word;
    std::optional<char32_t> lastKana;

    // For GAME_STOPPED
    std::optional<PlayerScore> score;
};
