#pragma once

#include <memory>

#include "game_session.hpp"
#include "idictionary.hpp"
#include "igame_fabric.hpp"
#include "igame_session.hpp"
#include "igames_repo.hpp"

using ull = unsigned long long;

class GameFabric : public IGameFabric
{
  public:
    std::shared_ptr<IGameSession> createGame(ull, ull) override;

    GameFabric(std::shared_ptr<IDictionary>, std::shared_ptr<IGamesRepo>);

  private:
    std::shared_ptr<IDictionary> dict_;
    std::shared_ptr<IGamesRepo> repo_;
};