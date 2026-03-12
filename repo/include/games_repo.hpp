#pragma once

#include <string>
#include <vector>

#include "game.hpp"
#include "igames_repo.hpp"

using ull = unsigned long long;

class GamesRepo : public IGamesRepo
{
  public:
    GamesRepo(std::string);
    void putGame(std::vector<Game>) override;
    std::vector<Game> getGameHistoryByUserId(ull) override;

  private:
    std::string dbPath_;
    void initTables();
};
