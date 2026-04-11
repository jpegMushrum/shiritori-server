#pragma once

#include <memory>
#include <vector>

#include "game_info.hpp"
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
    ull login(const std::string&) override;
    std::vector<GameInfo> getGamesHistory(ull) override;

    InfoService(std::shared_ptr<IUsersRepo>, std::shared_ptr<IGamesRepo>);

  private:
    std::shared_ptr<IUsersRepo> usersRepo_;
    std::shared_ptr<IGamesRepo> gamesRepo_;
};