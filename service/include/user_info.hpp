#pragma once

#include <iostream>

using ull = unsigned long long;

class UserInfo
{
  public:
    UserInfo(ull, std::string, int, int);

    ull id;
    std::string nickname;
    int games;
    int words;

    friend std::ostream& operator<<(std::ostream&, const UserInfo&);
};