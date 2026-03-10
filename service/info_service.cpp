#include <functional>
#include <memory>
#include <vector>

#include "info_service.hpp"
#include "mapper.hpp"
#include "user_info.hpp"

using ull = unsigned long long;

InfoService::InfoService(std::shared_ptr<UsersRepo> usersRepo, std::shared_ptr<GamesRepo> gamesRepo)
    : usersRepo_(usersRepo), gamesRepo_(gamesRepo)
{
}

UserInfo InfoService::getUserInfo(ull id)
{
    auto user = usersRepo_->getUser(id);

    return Mapper::UserToDto(user);
}

ull InfoService::addUser(const std::string& nickname)
{

    User newUser(0, nickname, 0, 0);
    ull id = usersRepo_->addUser(newUser);

    return id;
}
