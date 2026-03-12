#pragma once

#include <memory>

#include "game_session.hpp"
#include "games_repo.hpp"
#include "idictionary.hpp"
#include "igame_fabric.hpp"
#include "igame_session.hpp"

using ull = unsigned long long;

class GameFabric : public IGameFabric
{
  public:
    std::shared_ptr<IGameSession> createGame(ull, ull) override;

    GameFabric(std::shared_ptr<IDictionary>, std::shared_ptr<GamesRepo>);

  private:
    std::shared_ptr<IDictionary> dict_;
    std::shared_ptr<GamesRepo> repo_;
};