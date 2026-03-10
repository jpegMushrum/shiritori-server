#pragma once

#include <string>

using ull = unsigned long long;

class User
{
  public:
    User();
    User(ull, std::string, int, int);

    ull id;
    std::string nickname;
    int games;
    int words;
};