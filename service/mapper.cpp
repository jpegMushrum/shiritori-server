#include "mapper.hpp"

UserInfo Mapper::UserToDto(User user)
{
    return UserInfo(user.id, user.nickname, 0, 0);
}

GameInfo Mapper::GameToDto(Game game)
{
    return GameInfo(game.gameId, game.userId, game.words, game.place);
}

WordInfo Mapper::WordToDto(Word word)
{
    return WordInfo(word.kanji, word.readings, word.partsOfSpeach, word.meaning);
}