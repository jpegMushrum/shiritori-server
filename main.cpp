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

int main()
{
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);

    std::string dbPath = (std::filesystem::current_path() / "data" / "shiritoriDb.db").string();

    auto jisho = std::make_shared<JishoDict>();
    auto usersRepo = std::make_shared<UsersRepo>(dbPath);
    auto gamesRepo = std::make_shared<GamesRepo>(dbPath);

    auto infoService = std::make_unique<InfoService>(usersRepo, gamesRepo);
    auto gameFabric = std::make_unique<GameFabric>(jisho, gamesRepo);

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
                gamesCtr.getActiveGames(
                    [](std::vector<GameContext> gc)
                    {
                        for (int i = 0; i < gc.size(); i++)
                        {
                            std::cout << gc[i] << "\n";
                        }
                    });
            }
            catch (...)
            {
                std::cerr << "getActiveGames err: bad args\n";
            }

            continue;
        }

        if (command == "getGameInfo")
        {

            try
            {
                if (args.size() < 1)
                {
                    std::cerr << "searchWord err: bad args\n";
                    continue;
                }

                ull gameId = std::stoull(args[0]);

                gamesCtr.getGameInfo(gameId, [](GameContext gc) { std::cout << gc << "\n"; });
            }
            catch (...)
            {
                std::cerr << "getGameInfo err: bad args\n";
            }

            continue;
        }

        if (command == "startNewGame")
        {
            try
            {
                if (args.size() < 1)
                {
                    std::cerr << "searchWord err: bad args\n";
                    continue;
                }

                ull userId = std::stoull(args[0]);

                gamesCtr.startNewGame(userId, [](GameContext gc) { std::cout << gc << '\n'; });
            }
            catch (...)
            {
                std::cerr << "startNewGame err: bad args\n";
            }

            continue;
        }

        if (command == "stopGame")
        {
            try
            {
                if (args.size() < 2)
                {
                    std::cerr << "searchWord err: bad args\n";
                    continue;
                }

                ull gameId = std::stoull(args[0]);
                ull userId = std::stoull(args[1]);

                gamesCtr.stopGame(gameId, userId);
            }
            catch (...)
            {
                std::cerr << "startNewGame err: bad args\n";
            }

            continue;
        }

        if (command == "addPlayerToGame")
        {
            try
            {
                if (args.size() < 2)
                {
                    std::cerr << "searchWord err: bad args\n";
                    continue;
                }

                ull userId = std::stoull(args[0]);
                ull gameId = std::stoull(args[1]);

                gamesCtr.addPlayerToGame(userId, gameId);
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
                if (args.size() < 3)
                {
                    std::cerr << "searchWord err: bad args\n";
                    continue;
                }

                ull gameId = std::stoull(args[0]);
                ull userId = std::stoull(args[1]);
                std::string word = args[2];

                gamesCtr.handleWord(gameId, userId, word,
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