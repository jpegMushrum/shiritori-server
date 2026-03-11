#pragma once

#include <atomic>
#include <functional>
#include <map>
#include <memory>
#include <shared_mutex>
#include <string>
#include <vector>

#include "game_context.hpp"
#include "game_fabric.hpp"
#include "game_session.hpp"
#include "handle_word_status.hpp"
#include "task_queue.hpp"

using ull = unsigned long long;

class GamesController
{
  public:
    GamesController(std::shared_ptr<TaskQueue>, std::unique_ptr<GameFabric>);

    void startNewGame(ull, std::function<void(GameContext)>);
    void handleWord(ull, ull, std::string, std::function<void(HandleWordStatus)>);
    void getActiveGames(std::function<void(std::vector<GameContext>)>);
    void addPlayerToGame(ull, ull);
    void stopGame(ull, ull);
    void getGameInfo(ull, std::function<void(GameContext)>);

  private:
    std::unordered_map<ull, std::shared_ptr<GameSession>> activeGames_;
    std::shared_ptr<TaskQueue> taskQueue_;
    std::unique_ptr<GameFabric> gameFabric_;
    ull nextGameId_;

    mutable std::shared_mutex mu_;
};