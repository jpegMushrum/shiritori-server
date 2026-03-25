#pragma once

#include <atomic>
#include <functional>
#include <map>
#include <memory>
#include <shared_mutex>
#include <string>
#include <vector>

#include "game_context.hpp"
#include "handle_word_status.hpp"
#include "igame_fabric.hpp"
#include "igame_session.hpp"
#include "igames_controller.hpp"
#include "task_queue.hpp"

using ull = unsigned long long;

class GamesController : public IGamesController
{
  public:
    GamesController(std::shared_ptr<ITaskQueue>, std::unique_ptr<IGameFabric>);

    void startNewGame(ull, std::function<void(GameContext)>) override;
    void handleWord(ull, ull, std::string, std::function<void(HandleWordStatus)>) override;
    void getActiveGames(std::function<void(std::vector<GameContext>)>) override;
    void addPlayerToGame(ull, ull) override;
    void stopGame(ull, ull) override;
    void getGameInfo(ull, std::function<void(GameContext)>) override;

  private:
    std::unordered_map<ull, std::shared_ptr<IGameSession>> activeGames_;
    std::shared_ptr<ITaskQueue> taskQueue_;
    std::unique_ptr<IGameFabric> gameFabric_;
    ull nextGameId_;

    mutable std::shared_mutex mu_;
};