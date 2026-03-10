#pragma once

#include <string>
#include <vector>

#include "game.hpp"

using ull = unsigned long long;

class GamesRepo
{
  public:
    GamesRepo(std::string);
    void putGame(std::vector<Game>);
    std::vector<Game> getGameHistoryByUserId(ull);

  private:
    std::string dbPath_;
    void initTables();
};
