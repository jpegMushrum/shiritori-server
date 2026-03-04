#pragma once

#include <iostream>

using ull = unsigned long long;

class UserInfo {
public:
    ull id;
  
    friend std::ostream& operator<<(std::ostream&, const UserInfo&);
};