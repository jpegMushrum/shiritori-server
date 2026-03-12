#pragma once

#include <string>
#include <vector>

#include "game.hpp"

using ull = unsigned long long;

class IGamesRepo
{
  public:
    virtual ~IGamesRepo() = default;

    virtual void putGame(std::vector<Game>) = 0;
    virtual std::vector<Game> getGameHistoryByUserId(ull) = 0;
};
