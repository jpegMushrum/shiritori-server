#include "games_controller.hpp"

GamesController::GamesController(std::shared_ptr<ITaskQueue> taskQueue,
                                 std::unique_ptr<IGameFabric> gameFabric)
    : taskQueue_(taskQueue), nextGameId_(0), gameFabric_(std::move(gameFabric))
{
}

void GamesController::startNewGame(ull adminId, std::function<void(GameContext)> f)
{
    taskQueue_->addTask(
        [this, adminId, f]()
        {
            GameContext info;
            {
                std::unique_lock lock(mu_);

                ull id = nextGameId_++;

                activeGames_[id] = gameFabric_->createGame(id, adminId);
                info = activeGames_[id]->getInfo();
            }

            f(info);
        });
}

void GamesController::stopGame(ull gameId, ull userId)
{
    taskQueue_->addTask(
        [this, gameId, userId]()
        {
            std::unique_lock lock(mu_);

            auto it = activeGames_.find(gameId);
            if (it == activeGames_.end())
            {
                return;
            }

            if (it->second->getInfo().adminId == userId)
            {
                it->second->stopGame();
                activeGames_.erase(it);
            }
        });
}

void GamesController::handleWord(ull gameId, ull userId, std::string word,
                                 std::function<void(HandleWordStatus)> f)
{
    taskQueue_->addTask(
        [this, gameId, userId, word, f]()
        {
            std::shared_ptr<IGameSession> game;

            {
                std::shared_lock lock(mu_);

                auto it = activeGames_.find(gameId);
                if (it == activeGames_.end())
                {
                    f(HandleWordStatus::GAME_NOT_FOUND);
                    return;
                }

                game = it->second;
            }

            HandleWordStatus status = game->handleWord(userId, word);

            if (status == HandleWordStatus::GOT_END_WORD)
            {
                std::unique_lock lock(mu_);

                auto it = activeGames_.find(gameId);
                if (it != activeGames_.end())
                {
                    activeGames_.erase(it);
                }
            }

            f(status);
        });
}

void GamesController::getActiveGames(std::function<void(std::vector<GameContext>)> f)
{
    taskQueue_->addTask(
        [this, f]()
        {
            std::vector<GameContext> result;

            {
                std::shared_lock lock(mu_);

                auto it = activeGames_.begin();
                while (it != activeGames_.end())
                {
                    result.push_back(it->second->getInfo());
                    it++;
                }
            }

            f(result);
        });
}

void GamesController::getGameInfo(ull gameId, std::function<void(GameContext)> f)
{
    taskQueue_->addTask(
        [this, gameId, f]()
        {
            GameContext info;
            {
                std::shared_lock lock(mu_);

                auto it = activeGames_.find(gameId);
                if (it == activeGames_.end())
                {
                    return;
                }

                info = it->second->getInfo();
            }

            f(info);
        });
}

void GamesController::addPlayerToGame(ull userId, ull gameId, std::function<void(WordInfo)> update)
{
    taskQueue_->addTask(
        [this, userId, gameId, update]()
        {
            std::shared_lock lock(mu_);
            auto game = activeGames_.find(gameId);
            if (game == activeGames_.end())
            {
                return;
            }

            game->second->addUser(userId);
            game->second->subscribe(update);
        });
}