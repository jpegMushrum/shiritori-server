#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "game_context.hpp"
#include "game_update_event.hpp"
#include "games_repo.hpp"
#include "handle_word_status.hpp"
#include "idictionary.hpp"
#include "player_join_info.hpp"
#include "word_info.hpp"

using ull = unsigned long long;

class IGameSession
{
  public:
    virtual ~IGameSession() = default;

    virtual bool addUser(ull) = 0;
    virtual HandleWordStatus handleWord(ull, const std::string&) = 0;
    virtual void stopGame() = 0;
    virtual void subscribe(ull, std::function<void(const GameUpdateEvent&)>) = 0;

    virtual GameContext getInfo() = 0;
    virtual PlayerJoinInfo getPlayerJoinInfo() = 0;
};