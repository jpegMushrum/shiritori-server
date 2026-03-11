#pragma once

#include <memory>

#include "game_session.hpp"
#include "games_repo.hpp"
#include "idictionary.hpp"

using ull = unsigned long long;

class GameFabric
{
  public:
    std::shared_ptr<GameSession> createGame(ull, ull);

    GameFabric(std::shared_ptr<IDictionary>, std::shared_ptr<GamesRepo>);

  private:
    std::shared_ptr<IDictionary> dict_;
    std::shared_ptr<GamesRepo> repo_;
};