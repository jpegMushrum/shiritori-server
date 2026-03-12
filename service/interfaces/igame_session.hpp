#pragma once

#include <memory>
#include <string>

#include "game_context.hpp"
#include "games_repo.hpp"
#include "handle_word_status.hpp"
#include "idictionary.hpp"

using ull = unsigned long long;

class IGameSession
{
  public:
    virtual ~IGameSession() = default;

    virtual void addUser(ull) = 0;
    virtual HandleWordStatus handleWord(ull, const std::string&) = 0;
    virtual void stopGame() = 0;

    virtual GameContext GetInfo() = 0;
};