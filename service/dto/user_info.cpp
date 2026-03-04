#include <iostream>

#include "user_info.hpp"


std::ostream& operator<<(std::ostream& out, const UserInfo& pi) {
    out << pi.id;
    return out;
}