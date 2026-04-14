#pragma once

#include <functional>
#include <memory>
#include <string>

#include "game_context.hpp"
#include "games_repo.hpp"
#include "handle_word_status.hpp"
#include "idictionary.hpp"
#include "word_info.hpp"

using ull = unsigned long long;

class IGameSession
{
  public:
    virtual ~IGameSession() = default;

    virtual bool addUser(ull) = 0;
    virtual HandleWordStatus handleWord(ull, const std::string&) = 0;
    virtual void stopGame() = 0;
    virtual void subscribe(std::function<void(WordInfo)>) = 0;

    virtual GameContext getInfo() = 0;
};