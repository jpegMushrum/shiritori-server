#include <WinSock2.h>
#include <boost/asio.hpp>
#include <filesystem>
#include <memory>
#include <windows.h>

#include "game_fabric.hpp"
#include "games_controller.hpp"
#include "idictionary.hpp"
#include "info_controller.hpp"
#include "info_service.hpp"
#include "jisho.hpp"
#include "server.hpp"
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
    auto infoCtr = std::make_shared<InfoController>(taskQueue, std::move(infoService));
    auto gamesCtr = std::make_shared<GamesController>(taskQueue, std::move(gameFabric));

    boost::asio::io_context io;

    Server server(io, 3000, infoCtr, gamesCtr);
    server.startAccepting();
}