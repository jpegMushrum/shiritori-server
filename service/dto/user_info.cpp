#include <iostream>

#include "user_info.hpp"

using ull = unsigned long long;

UserInfo::UserInfo(ull id, std::string nickname, int games, int words)
    : id(id), nickname(nickname), games(games), words(words)
{
}

std::ostream& operator<<(std::ostream& out, const UserInfo& ui)
{
    double awg = 0;
    if (ui.games > 0)
    {
        awg = ((double)ui.words) / ui.games;
    }

    out << ui.id << " " << ui.nickname << " " << awg;
    return out;
}