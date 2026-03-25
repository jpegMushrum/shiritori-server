#include <iostream>
#include <sstream>

#include "router.hpp"

Router::Router(std::function<void(std::string)> writeCb, std::shared_ptr<IInfoController> infoCtr,
               std::shared_ptr<IGamesController> gamesCtr)
    : writeCb_(writeCb), infoCtr_(infoCtr), gamesCtr_(gamesCtr)
{
}

void Router::parseAndAnswer(std::string querry)
{
    std::stringstream ss(querry);
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
                writeCb_("Error: getUser requires user ID");
                return;
            }

            ull id = std::stoull(args[0]);

            auto writeCb = writeCb_;
            infoCtr_->getUserInfo(id, [writeCb](UserInfo ui) { writeCb(Router::uiToString(ui)); });
        }
        catch (...)
        {
            writeCb_("Error: invalid arguments for getUser");
        }

        return;
    }

    if (command == "addUser")
    {
        try
        {
            if (args.size() < 1)
            {
                writeCb_("Error: addUser requires nickname");
                return;
            }

            std::string nickname = args[0];

            auto writeCb = writeCb_;
            infoCtr_->addUser(nickname, [writeCb](ull userId) { writeCb(std::to_string(userId)); });
        }
        catch (...)
        {
            writeCb_("Error: invalid arguments for addUser");
        }

        return;
    }

    if (command == "getActiveGames")
    {
        try
        {
            auto writeCb = writeCb_;
            gamesCtr_->getActiveGames(
                [writeCb](std::vector<GameContext> gc)
                {
                    std::string ans = "";
                    for (int i = 0; i < gc.size(); i++)
                    {
                        ans += Router::gcToString(gc[i]) + ";";
                    }

                    if (!ans.empty())
                    {
                        ans.pop_back();
                    }

                    writeCb(ans);
                });
        }
        catch (...)
        {
            writeCb_("Error: getActiveGames failed");
        }

        return;
    }

    if (command == "getGameInfo")
    {
        try
        {
            if (args.size() < 1)
            {
                writeCb_("Error: getGameInfo requires game ID");
                return;
            }

            ull gameId = std::stoull(args[0]);

            auto writeCb = writeCb_;
            gamesCtr_->getGameInfo(gameId,
                                   [writeCb](GameContext gc) { writeCb(Router::gcToString(gc)); });
        }
        catch (...)
        {
            writeCb_("Error: invalid arguments for getGameInfo");
        }

        return;
    }

    if (command == "startNewGame")
    {
        try
        {
            if (args.size() < 1)
            {
                writeCb_("Error: startNewGame requires user ID");
                return;
            }

            ull userId = std::stoull(args[0]);

            auto writeCb = writeCb_;
            gamesCtr_->startNewGame(userId,
                                    [writeCb](GameContext gc) { writeCb(Router::gcToString(gc)); });
        }
        catch (...)
        {
            writeCb_("Error: invalid arguments for startNewGame");
        }

        return;
    }

    if (command == "stopGame")
    {
        try
        {
            if (args.size() < 2)
            {
                writeCb_("Error: stopGame requires game ID and user ID");
                return;
            }

            ull gameId = std::stoull(args[0]);
            ull userId = std::stoull(args[1]);

            gamesCtr_->stopGame(gameId, userId);
            writeCb_("Game stopped successfully");
        }
        catch (...)
        {
            writeCb_("Error: invalid arguments for stopGame");
        }

        return;
    }

    if (command == "addPlayerToGame")
    {
        try
        {
            if (args.size() < 2)
            {
                writeCb_("Error: addPlayerToGame requires user ID and game ID");
                return;
            }

            ull userId = std::stoull(args[0]);
            ull gameId = std::stoull(args[1]);

            gamesCtr_->addPlayerToGame(userId, gameId);
            writeCb_("Player added successfully");
        }
        catch (...)
        {
            writeCb_("Error: invalid arguments for addPlayerToGame");
        }

        return;
    }

    if (command == "handleWord")
    {
        try
        {
            if (args.size() < 3)
            {
                writeCb_("Error: handleWord requires game ID, user ID, and word");
                return;
            }

            ull gameId = std::stoull(args[0]);
            ull userId = std::stoull(args[1]);
            std::string word = args[2];

            auto writeCb = writeCb_;
            gamesCtr_->handleWord(gameId, userId, word, [writeCb](HandleWordStatus status)
                                  { writeCb(Router::statusToString(status)); });
        }
        catch (...)
        {
            writeCb_("Error: invalid arguments for handleWord");
        }

        return;
    }

    if (command == "getGamesHistory")
    {
        try
        {
            if (args.size() < 1)
            {
                writeCb_("Error: handleWord requires game ID, user ID, and word");
                return;
            }

            ull userId = std::stoull(args[0]);

            auto writeCb = writeCb_;
            infoCtr_->getGamesHistory(userId,
                                      [writeCb](std::vector<GameInfo> gi)
                                      {
                                          std::string ans = "";
                                          for (int i = 0; i < gi.size(); i++)
                                          {
                                              ans += Router::giToString(gi[i]) + ";";
                                          }

                                          if (!ans.empty())
                                          {
                                              ans.pop_back();
                                          }

                                          writeCb(ans);
                                      });
        }
        catch (...)
        {
            writeCb_("Error: invalid arguments for handleWord");
        }

        return;
    }

    writeCb_("Error: Unknown command");
}

std::string Router::uiToString(const UserInfo& ui)
{
    std::stringstream ss;
    ss << ui;
    return ss.str();
}

std::string Router::gcToString(const GameContext& gc)
{
    std::stringstream ss;
    ss << gc;
    return ss.str();
}

std::string Router::giToString(const GameInfo& gi)
{
    std::stringstream ss;
    ss << gi;
    return ss.str();
}

std::string Router::statusToString(HandleWordStatus status)
{
    switch (status)
    {
    case OK:
        return "OK";
    case GOT_ERROR:
        return "GOT_ERROR";
    case WRONG_ORDER:
        return "WRONG_ORDER";
    case NOT_JAPANESE_WORD:
        return "NOT_JAPANESE_WORD";
    case NO_SPEACH_PART:
        return "NO_SPEACH_PART";
    case NO_FOUND_WORD:
        return "NO_FOUND_WORD";
    case GOT_END_WORD:
        return "GOT_END_WORD";
    case GOT_DOUBLED_WORD:
        return "GOT_DOUBLED_WORD";
    case CANT_JOIN_WORDS:
        return "CANT_JOIN_WORDS";
    case GAME_NOT_FOUND:
        return "GAME_NOT_FOUND";
    case GAME_STOPPED:
        return "GAME_STOPPED";
    case NO_FOUND_PLAYER:
        return "NO_FOUND_PLAYER";
    default:
        return "UNKNOWN_STATUS";
    }
}
