#pragma once

#include <memory>

#include "igame_session.hpp"

using ull = unsigned long long;

class IGameFabric
{
  public:
    virtual ~IGameFabric() = default;
    virtual std::shared_ptr<IGameSession> createGame(ull, ull) = 0;
};