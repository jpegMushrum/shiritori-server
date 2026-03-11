#include "player_score.hpp"

bool PlayerScore::psCmp(const PlayerScore& a, const PlayerScore& b)
{
    return a.score > b.score;
}