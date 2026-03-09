#pragma once

#include <memory>

#include "game_session.hpp"
#include "idictionary.hpp"

using ull = unsigned long long;

class GameFabric
{
  public:
    std::shared_ptr<GameSession> createGame(ull);

    GameFabric(std::shared_ptr<IDictionary>);

  private:
    std::shared_ptr<IDictionary> dict_;
};