#pragma once

#include <string>

#include "user.hpp"

using ull = unsigned long long;

class IUsersRepo
{
  public:
    virtual ~IUsersRepo() = default;

    virtual ull addUser(User user) = 0;
    virtual void changeUser(User user) = 0;
    virtual User getUser(ull id) = 0;
};