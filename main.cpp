#include <iostream>
#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <memory>

#include "info_service.hpp"
#include "users_repo.hpp"
#include "user_info.hpp"

using ull = unsigned long long;

int main() {

    UsersRepo usersRepo;
    InfoService infoService(usersRepo);

    while (true) {
        std::string in;
        std::getline(std::cin, in);

        std::stringstream ss(in);
        std::string command;
        std::getline(ss, command, ' ');
    
        std::vector<std::string> args;
        std::string t;
        while (std::getline(ss, t, ' ')) {
            args.push_back(t);
        }


        if (command == "getUser") {
            try {
                ull id = std::stoull(args[0]);
                UserInfo result = infoService.getUserInfo(id);
                std::cout << result << "\n";
            }
            catch (...) {
                std::cerr << "getPlayer err: bad args";    
            }
            
            continue;
        }

        std::cerr << "main err: Unknown command\n";
    }
}