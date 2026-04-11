#pragma once

#include <string>

#include "iusers_repo.hpp"
#include "user.hpp"

using ull = unsigned long long;

class UsersRepo : public IUsersRepo
{
  public:
    UsersRepo(std::string);

    ull addUser(User user) override;
    void changeUser(User user) override;
    User getUser(ull id) override;
    User getUserByNickname(const std::string& nickname) override;

  private:
    std::string dbPath_;
    void initTables();
};