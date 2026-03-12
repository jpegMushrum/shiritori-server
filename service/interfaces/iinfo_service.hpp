#pragma once

#include <memory>
#include <vector>

#include "games_repo.hpp"
#include "user_info.hpp"
#include "users_repo.hpp"

using ull = unsigned long long;

class IInfoService
{
  public:
    virtual ~IInfoService() = default;

    virtual UserInfo getUserInfo(ull) = 0;
    virtual ull addUser(const std::string&) = 0;
};