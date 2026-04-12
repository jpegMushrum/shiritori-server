#include <format>
#include <iostream>
#include <nlohmann/json.hpp>
#include <sstream>

#include "router.hpp"

using json = nlohmann::json;

std::shared_ptr<SessionManager> Router::sessionManager_ = std::make_shared<SessionManager>();

Router::Router(std::function<void(std::string)> writeCb, std::shared_ptr<IInfoController> infoCtr,
               std::shared_ptr<IGamesController> gamesCtr)
    : writeCb_(writeCb), infoCtr_(infoCtr), gamesCtr_(gamesCtr)
{
}

void Router::parseAndAnswer(std::string querry)
{
    if (querry.empty())
    {
        writeCb_("Error: empty querry");
        return;
    }

    std::stringstream ss(querry);
    std::string command;
    std::string requestId;

    std::vector<std::string> args;
    std::string t;

    try
    {
        std::getline(ss, requestId, ' ');
        std::getline(ss, command, ' ');

        while (std::getline(ss, t, ' '))
        {
            args.push_back(t);
        }
    }
    catch (...)
    {
        writeCb_("Error: bad args");
        return;
    }

    if (requestId.empty() || command.empty())
    {
        writeCb_("Error: bad args");
        return;
    }

    auto writeCb = writeCb_;
    auto writeRId = [writeCb, requestId](std::string stmt)
    {
        std::string msg = requestId + " " + stmt;
        writeCb(msg);
    };

    if (command == "getUserInfo")
    {
        try
        {
            if (args.empty())
            {
                writeRId("Error: getUserInfo requires session ID");
                return;
            }

            std::string sessionId = args[0];

            if (!sessionManager_->sessionExists(sessionId))
            {
                writeRId("Error: Invalid session ID");
                return;
            }

            ull userId = sessionManager_->getUserIdFromSession(sessionId);

            infoCtr_->getUserInfo(userId,
                                  [writeRId](UserInfo ui) { writeRId(Router::uiToString(ui)); });
        }
        catch (...)
        {
            writeRId("Error: invalid arguments for getUserInfo");
        }

        return;
    }

    if (command == "addUser")
    {
        try
        {
            if (args.size() < 1)
            {
                writeRId("Error: addUser requires nickname");
                return;
            }

            std::string nickname = args[0];

            infoCtr_->addUser(nickname,
                              [writeRId](ull userId) { writeRId(std::to_string(userId)); });
        }
        catch (...)
        {
            writeRId("Error: invalid arguments for addUser");
        }

        return;
    }

    if (command == "login")
    {
        try
        {
            if (args.size() < 1)
            {
                writeRId("Error: login requires username");
                return;
            }

            std::string username = args[0];

            auto sessionManager = sessionManager_;
            infoCtr_->login(username,
                            [writeRId, sessionManager](ull userId)
                            {
                                std::string sessionId = sessionManager->createSession(userId);
                                writeRId(sessionId);
                            });
        }
        catch (...)
        {
            writeRId("Error: invalid arguments for login");
        }

        return;
    }

    if (command == "logout")
    {
        try
        {
            if (args.size() < 1)
            {
                writeRId("Error: logout requires session ID");
                return;
            }

            std::string sessionId = args[0];

            if (!sessionManager_->sessionExists(sessionId))
            {
                writeRId("Error: Invalid session ID");
                return;
            }

            sessionManager_->removeSession(sessionId);
            writeRId("Logged out successfully");
        }
        catch (...)
        {
            writeRId("Error: invalid arguments for logout");
        }

        return;
    }

    if (command == "getActiveGames")
    {
        try
        {
            gamesCtr_->getActiveGames(
                [writeRId](std::vector<GameContext> gc)
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

                    writeRId(ans);
                });
        }
        catch (...)
        {
            writeRId("Error: getActiveGames failed");
        }

        return;
    }

    if (command == "getGameInfo")
    {
        try
        {
            if (args.size() < 1)
            {
                writeRId("Error: getGameInfo requires game ID");
                return;
            }

            ull gameId = std::stoull(args[0]);

            gamesCtr_->getGameInfo(gameId, [writeRId](GameContext gc)
                                   { writeRId(Router::gcToString(gc)); });
        }
        catch (...)
        {
            writeRId("Error: invalid arguments for getGameInfo");
        }

        return;
    }

    if (command == "startNewGame")
    {
        try
        {
            if (args.size() < 1)
            {
                writeRId("Error: startNewGame requires session ID");
                return;
            }

            std::string sessionId = args[0];

            if (!sessionManager_->sessionExists(sessionId))
            {
                writeRId("Error: Invalid session ID");
                return;
            }

            ull userId = sessionManager_->getUserIdFromSession(sessionId);

            gamesCtr_->startNewGame(userId, [writeRId](GameContext gc)
                                    { writeRId(Router::gcToString(gc)); });
        }
        catch (...)
        {
            writeRId("Error: invalid arguments for startNewGame");
        }

        return;
    }

    if (command == "stopGame")
    {
        try
        {
            if (args.size() < 2)
            {
                writeRId("Error: stopGame requires session ID and game ID");
                return;
            }

            std::string sessionId = args[0];
            ull gameId = std::stoull(args[1]);

            if (!sessionManager_->sessionExists(sessionId))
            {
                writeRId("Error: Invalid session ID");
                return;
            }

            ull userId = sessionManager_->getUserIdFromSession(sessionId);

            gamesCtr_->stopGame(gameId, userId);
            writeRId("Game stopped successfully");
        }
        catch (...)
        {
            writeRId("Error: invalid arguments for stopGame");
        }

        return;
    }

    if (command == "addPlayerToGame")
    {
        try
        {
            if (args.size() < 2)
            {
                writeRId("Error: addPlayerToGame requires session ID and game ID");
                return;
            }

            std::string sessionId = args[0];
            ull gameId = std::stoull(args[1]);

            if (!sessionManager_->sessionExists(sessionId))
            {
                writeRId("Error: Invalid session ID");
                return;
            }

            ull userId = sessionManager_->getUserIdFromSession(sessionId);

            gamesCtr_->addPlayerToGame(userId, gameId,
                                       [writeRId, gameId](WordInfo wi)
                                       {
                                           std::string msg = std::format("NewWord {} {}", gameId,
                                                                         Router::wiToString(wi));
                                           writeRId(msg);
                                       });
            writeRId("Player added successfully");
        }
        catch (...)
        {
            writeRId("Error: invalid arguments for addPlayerToGame");
        }

        return;
    }

    if (command == "handleWord")
    {
        try
        {
            if (args.size() < 3)
            {
                writeRId("Error: handleWord requires game ID, session ID, and word");
                return;
            }

            ull gameId = std::stoull(args[0]);
            std::string sessionId = args[1];
            std::string word = args[2];

            if (!sessionManager_->sessionExists(sessionId))
            {
                writeRId("Error: Invalid session ID");
                return;
            }

            ull userId = sessionManager_->getUserIdFromSession(sessionId);

            gamesCtr_->handleWord(gameId, userId, word, [writeRId](HandleWordStatus status)
                                  { writeRId(Router::statusToString(status)); });
        }
        catch (...)
        {
            writeRId("Error: invalid arguments for handleWord");
        }

        return;
    }

    if (command == "getGamesHistory")
    {
        try
        {
            if (args.size() < 1)
            {
                writeRId("Error: getGamesHistory requires session ID");
                return;
            }

            std::string sessionId = args[0];

            if (!sessionManager_->sessionExists(sessionId))
            {
                writeRId("Error: Invalid session ID");
                return;
            }

            ull userId = sessionManager_->getUserIdFromSession(sessionId);

            infoCtr_->getGamesHistory(userId,
                                      [writeRId](std::vector<GameInfo> gi)
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

                                          writeRId(ans);
                                      });
        }
        catch (...)
        {
            writeRId("Error: invalid arguments for getGamesHistory");
        }

        return;
    }

    writeRId("Error: Unknown command");
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

std::string Router::wiToString(const WordInfo& wi)
{
    json wiJson = {{"kanji", wi.kanji},
                   {"readings", wi.readings},
                   {"partsOfSpeach", wi.partsOfSpeach},
                   {"meaning", wi.meaning}};
    return wiJson.dump();
}