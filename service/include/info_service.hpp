#pragma once

#include <memory>
#include <vector>

#include "games_repo.hpp"
#include "iinfo_service.hpp"
#include "user_info.hpp"
#include "users_repo.hpp"

using ull = unsigned long long;

class InfoService : public IInfoService
{
  public:
    UserInfo getUserInfo(ull) override;
    ull addUser(const std::string&) override;

    InfoService(std::shared_ptr<UsersRepo>, std::shared_ptr<GamesRepo>);

  private:
    std::shared_ptr<UsersRepo> usersRepo_;
    std::shared_ptr<GamesRepo> gamesRepo_;
};