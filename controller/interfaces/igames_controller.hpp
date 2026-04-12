#pragma once

#include <functional>
#include <string>
#include <vector>

#include "game_context.hpp"
#include "handle_word_status.hpp"
#include "word_info.hpp"

using ull = unsigned long long;

class IGamesController
{
  public:
    virtual ~IGamesController() = default;

    virtual void startNewGame(ull, std::function<void(GameContext)>) = 0;
    virtual void handleWord(ull, ull, std::string, std::function<void(HandleWordStatus)>) = 0;
    virtual void getActiveGames(std::function<void(std::vector<GameContext>)>) = 0;
    virtual void addPlayerToGame(ull, ull, std::function<void(WordInfo)>) = 0;
    virtual void stopGame(ull, ull) = 0;
    virtual void getGameInfo(ull, std::function<void(GameContext)>) = 0;
};
