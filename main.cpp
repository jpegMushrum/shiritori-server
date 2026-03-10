#include <filesystem>
#include <functional>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>
#include <windows.h>

#include "game_fabric.hpp"
#include "games_controller.hpp"
#include "idictionary.hpp"
#include "info_controller.hpp"
#include "info_service.hpp"
#include "jisho.hpp"
#include "task_queue.hpp"
#include "user_info.hpp"
#include "users_repo.hpp"

using ull = unsigned long long;

/*
    Планы на сегодня:
        - (Done) Исправить модель юзера на количество игр и кол-во слов вместо awg
        - Сделать репозиторий игр
        - Распарсить ответ от jisho
        - Добавить игровую логику
*/

int main()
{
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);

    std::string dbPath = (std::filesystem::current_path() / "data" / "shiritoriDb.db").string();

    auto jisho = std::make_shared<JishoDict>();
    std::cout << "d1| \n";
    auto usersRepo = std::make_shared<UsersRepo>(dbPath);
    std::cout << "d2| \n";
    auto gamesRepo = std::make_shared<GamesRepo>();

    auto infoService = std::make_unique<InfoService>(usersRepo, gamesRepo);
    auto gameFabric = std::make_unique<GameFabric>(jisho);

    std::cout << "d3| \n";
    auto taskQueue = std::make_shared<TaskQueue>(std::thread::hardware_concurrency());
    InfoController infoCtr(taskQueue, std::move(infoService));
    GamesController gamesCtr(taskQueue, std::move(gameFabric));

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

        if (command == "addUser")
        {
            try
            {
                if (args.size() < 1)
                {
                    std::cerr << "addUser err: no args\n";
                    continue;
                }

                std::string nickname = args[0];

                infoCtr.addUser(nickname, [](ull userId) { std::cout << userId << '\n'; });
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

        if (command == "handleWord")
        {
            try
            {
                if (args.empty())
                {
                    std::cerr << "searchWord err: no args\n";
                    continue;
                }

                ull userId = std::stoull(args[0]);
                ull gameId = std::stoull(args[1]);
                std::string word = args[2];

                gamesCtr.HandleWord(userId, gameId, word,
                                    [](HandleWordStatus status) { std::cout << status << '\n'; });
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