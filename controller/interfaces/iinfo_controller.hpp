#pragma once

#include <functional>
#include <string>
#include <vector>

#include "game_info.hpp"
#include "user_info.hpp"

using ull = unsigned long long;

class IInfoController
{
  public:
    virtual ~IInfoController() = default;

    virtual void getUserInfo(ull, std::function<void(UserInfo)>) = 0;
    virtual void addUser(const std::string&, std::function<void(ull)>) = 0;
    virtual void getGamesHistory(ull, std::function<void(std::vector<GameInfo>)>) = 0;
};