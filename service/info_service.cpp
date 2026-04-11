#include <functional>
#include <memory>
#include <vector>

#include "game_info.hpp"
#include "info_service.hpp"
#include "mapper.hpp"
#include "user_info.hpp"

using ull = unsigned long long;

InfoService::InfoService(std::shared_ptr<IUsersRepo> usersRepo,
                         std::shared_ptr<IGamesRepo> gamesRepo)
    : usersRepo_(usersRepo), gamesRepo_(gamesRepo)
{
}

UserInfo InfoService::getUserInfo(ull id)
{
    auto user = usersRepo_->getUser(id);
    UserInfo ui = Mapper::UserToDto(user);

    auto gamesHistory = gamesRepo_->getGameHistoryByUserId(id);
    for (auto it = gamesHistory.begin(); it != gamesHistory.end(); it++)
    {
        ui.games += 1;
        ui.words += it->words;
    }

    return ui;
}

ull InfoService::addUser(const std::string& nickname)
{
    User newUser(0, nickname);
    ull id = usersRepo_->addUser(newUser);

    return id;
}

ull InfoService::login(const std::string& username)
{
    try
    {
        auto user = usersRepo_->getUserByNickname(username);
        return user.id;
    }
    catch (...)
    {
        User newUser(0, username);
        ull id = usersRepo_->addUser(newUser);
        return id;
    }
}

std::vector<GameInfo> InfoService::getGamesHistory(ull userId)
{
    auto history = gamesRepo_->getGameHistoryByUserId(userId);
    std::vector<GameInfo> result(history.size());

    for (int i = 0; i < history.size(); i++)
    {
        result[i] = Mapper::GameToDto(history[i]);
    }

    return result;
}