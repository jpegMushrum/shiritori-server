#include "info_controller.hpp"

InfoController::InfoController(std::shared_ptr<ITaskQueue> queue,
                               std::unique_ptr<IInfoService> infoService)
    : taskQueue_(queue), infoService_(std::move(infoService))
{
}

void InfoController::getUserInfo(ull id, std::function<void(UserInfo)> f)
{
    taskQueue_->addTask(
        [this, id, f]()
        {
            auto user = infoService_->getUserInfo(id);
            f(user);
        });
}

void InfoController::addUser(const std::string& nickname, std::function<void(ull)> f)
{
    taskQueue_->addTask(
        [this, nickname, f]()
        {
            try
            {
                auto userId = infoService_->addUser(nickname);
                f(userId);
            }
            catch (std::runtime_error e)
            {
                std::cerr << e.what() << '\n';
            }
        });
}

void InfoController::login(const std::string& username, std::function<void(ull)> f)
{
    taskQueue_->addTask(
        [this, username, f]()
        {
            try
            {
                auto userId = infoService_->login(username);
                f(userId);
            }
            catch (std::runtime_error e)
            {
                std::cerr << e.what() << '\n';
            }
        });
}

void InfoController::getGamesHistory(ull id, std::function<void(std::vector<GameInfo>)> f)
{
    taskQueue_->addTask(
        [this, id, f]()
        {
            auto history = infoService_->getGamesHistory(id);
            f(history);
        });
}