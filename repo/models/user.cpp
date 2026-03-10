#include "user.hpp"

using ull = unsigned long long;

User::User() : id(0), nickname(""), games(0), words(0) {}

User::User(ull id, std::string nickname, int games, int words)
    : id(id), nickname(nickname), games(games), words(words)
{
}