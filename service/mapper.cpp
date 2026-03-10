#include "mapper.hpp"
#include "game.hpp"
#include "game_info.hpp"
#include "user.hpp"
#include "user_info.hpp"

UserInfo Mapper::UserToDto(User user)
{
    return UserInfo(user.id, user.nickname, user.games, user.words);
}

GameInfo Mapper::GameToDto(Game game)
{
    return GameInfo(game.id, 0, 0, 16);
}