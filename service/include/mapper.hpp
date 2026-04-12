#pragma once

#include <vector>

#include "game.hpp"
#include "game_info.hpp"
#include "user.hpp"
#include "user_info.hpp"
#include "word.hpp"
#include "word_info.hpp"

class Mapper
{
  public:
    Mapper() = delete;

    static UserInfo UserToDto(User);
    static GameInfo GameToDto(Game);
    static WordInfo WordToDto(Word);
};