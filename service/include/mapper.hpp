#pragma once

#include <vector>

#include "game.hpp"
#include "game_info.hpp"
#include "user.hpp"
#include "user_info.hpp"

class Mapper
{
  public:
    Mapper() = delete;

    static UserInfo UserToDto(User);
    static GameInfo GameToDto(std::vector<Game>);
};