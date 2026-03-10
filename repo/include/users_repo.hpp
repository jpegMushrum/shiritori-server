#pragma once

#include <string>

#include "user.hpp"

using ull = unsigned long long;

class UsersRepo
{
  public:
    UsersRepo(std::string);

    ull addUser(User user);
    void changeUser(User user);
    User getUser(ull id);

  private:
    std::string dbPath_;
    void initTables();
};