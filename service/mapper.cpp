#include "mapper.hpp"
#include "game.hpp"
#include "game_info.hpp"
#include "user.hpp"
#include "user_info.hpp"

UserInfo Mapper::UserToDto(User user)
{
    return UserInfo(user.id, user.nickname, user.games, user.words);
}

GameInfo Mapper::GameToDto(std::vector<Game> gameParts)
{
    int wordsCount = 0, playersCount = 0;
    ull id = gameParts[0].gameId;
    for (auto game : gameParts)
    {
        wordsCount += game.words;
        playersCount++;
    }

    return GameInfo(id, playersCount, wordsCount);
}