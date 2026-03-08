#include <functional>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "games_controller.hpp"
#include "info_controller.hpp"
#include "info_service.hpp"
#include "jisho.hpp"
#include "task_queue.hpp"
#include "user_info.hpp"
#include "users_repo.hpp"
#include <windows.h>

using ull = unsigned long long;

int main()
{
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);

    auto dict = JishoDict();
    auto usersRepo = std::make_shared<UsersRepo>();
    auto gamesRepo = std::make_shared<GamesRepo>();
    auto infoService = std::make_unique<InfoService>(usersRepo, gamesRepo);

    auto taskQueue = std::make_shared<TaskQueue>(std::thread::hardware_concurrency());
    InfoController infoCtr(taskQueue, std::move(infoService));

    GamesController gamesCtr(taskQueue);

    while (true)
    {
        std::string in;
        std::getline(std::cin, in);

        std::stringstream ss(in);
        std::string command;
        std::getline(ss, command, ' ');

        std::vector<std::string> args;
        std::string t;
        while (std::getline(ss, t, ' '))
        {
            args.push_back(t);
        }

        if (command == "getUser")
        {
            try
            {
                if (args.empty())
                {
                    std::cerr << "getUser err: no args\n";
                    continue;
                }

                ull id = std::stoull(args[0]);

                infoCtr.getUserInfo(id, [](UserInfo ui) { std::cout << ui << '\n'; });
            }
            catch (...)
            {
                std::cerr << "getUser err: bad args\n";
            }

            continue;
        }

        if (command == "getActiveGames")
        {
            try
            {
                gamesCtr.GetActiveGames(
                    [](std::vector<GameInfo> gi)
                    {
                        for (int i = 0; i < gi.size(); i++)
                        {
                            std::cout << gi[i] << "\n";
                        }
                    });
            }
            catch (...)
            {
                std::cerr << "getActiveGames err: bad args\n";
            }

            continue;
        }

        if (command == "startNewGame")
        {
            try
            {
                gamesCtr.StartNewGame([](ull gameId) { std::cout << gameId << '\n'; });
            }
            catch (...)
            {
                std::cerr << "startNewGame err: bad args\n";
            }

            continue;
        }

        if (command == "searchWord")
        {
            try
            {
                if (args.empty())
                {
                    std::cerr << "searchWord err: no args\n";
                    continue;
                }

                std::string word = args[0];
                std::cout << "dmain| " << word << '\n';

                dict.SearchWord(word);
            }
            catch (...)
            {
                std::cerr << "searchWord err: bad args\n";
            }

            continue;
        }

        std::cerr << "main err: Unknown command\n";
    }
}