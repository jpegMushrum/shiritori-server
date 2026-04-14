#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <future>
#include <memory>
#include <string>

#include "game_context.hpp"
#include "handle_word_status.hpp"
#include "igames_controller.hpp"
#include "iinfo_controller.hpp"
#include "router.hpp"
#include "user_info.hpp"
#include "word_info.hpp"

using ::testing::_;
using ::testing::Invoke;
using ::testing::Matcher;
using ::testing::Return;

using ull = unsigned long long;

class MockInfoController : public IInfoController
{
  public:
    MOCK_METHOD(void, getUserInfo, (ull, std::function<void(UserInfo)>), (override));
    MOCK_METHOD(void, addUser, (const std::string&, std::function<void(ull)>), (override));
    MOCK_METHOD(void, getGamesHistory, (ull, std::function<void(std::vector<GameInfo>)>),
                (override));
    MOCK_METHOD(void, login, (const std::string&, std::function<void(ull)>), (override));
};

class MockGamesController : public IGamesController
{
  public:
    MOCK_METHOD(void, startNewGame, (ull, std::function<void(GameContext)>), (override));
    MOCK_METHOD(void, handleWord, (ull, ull, std::string, std::function<void(HandleWordStatus)>),
                (override));
    MOCK_METHOD(void, getActiveGames, (std::function<void(std::vector<GameContext>)>), (override));
    MOCK_METHOD(void, addPlayerToGame, (ull, ull, std::function<void(WordInfo)>), (override));
    MOCK_METHOD(void, stopGame, (ull, ull), (override));
    MOCK_METHOD(void, getGameInfo, (ull, std::function<void(GameContext)>), (override));
};

class RouterTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        mockInfoController = std::make_shared<MockInfoController>();
        mockGamesController = std::make_shared<MockGamesController>();

        responseMessages.clear();
        requestIdCounter = 0;
        auto writeCallback = [this](std::string msg) { responseMessages.push_back(msg); };

        router = std::make_unique<Router>(writeCallback, mockInfoController, mockGamesController);
    }

    int getNextRequestId()
    {
        return ++requestIdCounter;
    }

    std::string extractResponse(const std::string& fullMessage)
    {
        size_t spacePos = fullMessage.find(' ');
        if (spacePos != std::string::npos)
        {
            return fullMessage.substr(spacePos + 1);
        }
        return fullMessage;
    }

    std::string getValidSessionId()
    {
        ull testUserId = 999;
        EXPECT_CALL(*mockInfoController, login("testuser", _))
            .WillOnce(
                Invoke([testUserId](const std::string& username, std::function<void(ull)> callback)
                       { callback(testUserId); }));

        responseMessages.clear();
        int rid = getNextRequestId();
        router->parseAndAnswer(std::to_string(rid) + " login testuser");

        if (responseMessages.size() > 0)
        {
            return extractResponse(responseMessages[0]);
        }
        return "";
    }

    std::shared_ptr<MockInfoController> mockInfoController;
    std::shared_ptr<MockGamesController> mockGamesController;
    std::unique_ptr<Router> router;
    std::vector<std::string> responseMessages;
    int requestIdCounter;
};

TEST_F(RouterTest, GetUserSuccessfully)
{
    ull userId = 999;
    UserInfo expectedUser(userId, "Alice", 10, 25);
    std::string sessionId = getValidSessionId();

    EXPECT_CALL(*mockInfoController, getUserInfo(userId, _))
        .Times(1)
        .WillOnce(Invoke([expectedUser](ull id, std::function<void(UserInfo)> callback)
                         { callback(expectedUser); }));

    responseMessages.clear();
    int rid = getNextRequestId();
    router->parseAndAnswer(std::to_string(rid) + " getUserInfo " + sessionId);

    EXPECT_EQ(responseMessages.size(), 1);
    EXPECT_NE(responseMessages[0].find("999"), std::string::npos);
    EXPECT_NE(responseMessages[0].find("Alice"), std::string::npos);
}

TEST_F(RouterTest, GetUserWithoutIdArgument)
{
    EXPECT_CALL(*mockInfoController, getUserInfo(_, _)).Times(0);

    int rid = getNextRequestId();
    router->parseAndAnswer(std::to_string(rid) + " getUserInfo");

    EXPECT_EQ(responseMessages.size(), 1);
    EXPECT_EQ(responseMessages[0], std::to_string(rid) + " Error: getUserInfo requires session ID");
}

TEST_F(RouterTest, GetUserWithInvalidId)
{
    EXPECT_CALL(*mockInfoController, getUserInfo(_, _)).Times(0);

    int rid = getNextRequestId();
    router->parseAndAnswer(std::to_string(rid) + " getUserInfo invalid_id");

    EXPECT_EQ(responseMessages.size(), 1);
    EXPECT_EQ(responseMessages[0], std::to_string(rid) + " Error: Invalid session ID");
}

TEST_F(RouterTest, AddUserSuccessfully)
{
    std::string nickname = "Bob";
    ull newUserId = 100;

    EXPECT_CALL(*mockInfoController, addUser(nickname, _))
        .Times(1)
        .WillOnce(Invoke([newUserId](const std::string& nick, std::function<void(ull)> callback)
                         { callback(newUserId); }));

    int rid = getNextRequestId();
    router->parseAndAnswer(std::to_string(rid) + " addUser Bob");

    EXPECT_EQ(responseMessages.size(), 1);
    EXPECT_EQ(responseMessages[0], std::to_string(rid) + " 100");
}

TEST_F(RouterTest, AddUserWithoutNickname)
{
    EXPECT_CALL(*mockInfoController, addUser(_, _)).Times(0);

    int rid = getNextRequestId();
    router->parseAndAnswer(std::to_string(rid) + " addUser");

    EXPECT_EQ(responseMessages.size(), 1);
    EXPECT_EQ(responseMessages[0], std::to_string(rid) + " Error: addUser requires nickname");
}

TEST_F(RouterTest, GetActiveGamesWithMultipleGames)
{
    std::vector<GameContext> activeGames;
    activeGames.push_back(GameContext(1, 5, 2, 100, U'あ'));
    activeGames.push_back(GameContext(2, 8, 3, 101, U'い'));

    EXPECT_CALL(*mockGamesController, getActiveGames(_))
        .Times(1)
        .WillOnce(Invoke([activeGames](std::function<void(std::vector<GameContext>)> callback)
                         { callback(activeGames); }));

    int rid = getNextRequestId();
    router->parseAndAnswer(std::to_string(rid) + " getActiveGames");

    EXPECT_EQ(responseMessages.size(), 1);
    EXPECT_NE(responseMessages[0].find(";"), std::string::npos);
}

TEST_F(RouterTest, GetActiveGamesWithNoGames)
{
    std::vector<GameContext> emptyGames;

    EXPECT_CALL(*mockGamesController, getActiveGames(_))
        .Times(1)
        .WillOnce(
            Invoke([](std::function<void(std::vector<GameContext>)> callback) { callback({}); }));

    int rid = getNextRequestId();
    router->parseAndAnswer(std::to_string(rid) + " getActiveGames");

    EXPECT_EQ(responseMessages.size(), 1);
    EXPECT_EQ(responseMessages[0], std::to_string(rid) + " ");
}

TEST_F(RouterTest, GetGameInfoSuccessfully)
{
    ull gameId = 42;
    GameContext gameContext(gameId, 10, 2, 50, U'あ');

    EXPECT_CALL(*mockGamesController, getGameInfo(gameId, _))
        .Times(1)
        .WillOnce(Invoke([gameContext](ull id, std::function<void(GameContext)> callback)
                         { callback(gameContext); }));

    int rid = getNextRequestId();
    router->parseAndAnswer(std::to_string(rid) + " getGameInfo 42");

    EXPECT_EQ(responseMessages.size(), 1);
    EXPECT_NE(responseMessages[0].find("42"), std::string::npos);
}

TEST_F(RouterTest, GetGameInfoWithoutGameId)
{
    EXPECT_CALL(*mockGamesController, getGameInfo(_, _)).Times(0);

    int rid = getNextRequestId();
    router->parseAndAnswer(std::to_string(rid) + " getGameInfo");

    EXPECT_EQ(responseMessages.size(), 1);
    EXPECT_EQ(responseMessages[0], std::to_string(rid) + " Error: getGameInfo requires game ID");
}

TEST_F(RouterTest, GetGameInfoWithInvalidGameId)
{
    EXPECT_CALL(*mockGamesController, getGameInfo(_, _)).Times(0);

    int rid = getNextRequestId();
    router->parseAndAnswer(std::to_string(rid) + " getGameInfo not_a_number");

    EXPECT_EQ(responseMessages.size(), 1);
    EXPECT_EQ(responseMessages[0],
              std::to_string(rid) + " Error: invalid arguments for getGameInfo");
}

TEST_F(RouterTest, StartNewGameSuccessfully)
{
    ull userId = 999;
    GameContext newGame(1, 0, 1, userId, U'あ');
    std::string sessionId = getValidSessionId();

    EXPECT_CALL(*mockGamesController, startNewGame(userId, _))
        .Times(1)
        .WillOnce(Invoke([newGame](ull id, std::function<void(GameContext)> callback)
                         { callback(newGame); }));

    responseMessages.clear();
    int rid = getNextRequestId();
    router->parseAndAnswer(std::to_string(rid) + " startNewGame " + sessionId);

    EXPECT_EQ(responseMessages.size(), 1);
    EXPECT_NE(responseMessages[0].find("1"), std::string::npos);
}

TEST_F(RouterTest, StartNewGameWithoutSessionId)
{
    EXPECT_CALL(*mockGamesController, startNewGame(_, _)).Times(0);

    int rid = getNextRequestId();
    router->parseAndAnswer(std::to_string(rid) + " startNewGame");

    EXPECT_EQ(responseMessages.size(), 1);
    EXPECT_EQ(responseMessages[0],
              std::to_string(rid) + " Error: startNewGame requires session ID");
}

TEST_F(RouterTest, StartNewGameWithInvalidSessionId)
{
    EXPECT_CALL(*mockGamesController, startNewGame(_, _)).Times(0);

    int rid = getNextRequestId();
    router->parseAndAnswer(std::to_string(rid) + " startNewGame abc");

    EXPECT_EQ(responseMessages.size(), 1);
    EXPECT_EQ(responseMessages[0], std::to_string(rid) + " Error: Invalid session ID");
}

TEST_F(RouterTest, StopGameSuccessfully)
{
    ull gameId = 5;
    ull userId = 999;
    std::string sessionId = getValidSessionId();

    EXPECT_CALL(*mockGamesController, stopGame(gameId, userId)).Times(1);

    responseMessages.clear();
    int rid = getNextRequestId();
    router->parseAndAnswer(std::to_string(rid) + " stopGame " + sessionId + " 5");

    EXPECT_EQ(responseMessages.size(), 1);
    EXPECT_EQ(responseMessages[0], std::to_string(rid) + " Game stopped successfully");
}

TEST_F(RouterTest, StopGameWithMissingGameId)
{
    EXPECT_CALL(*mockGamesController, stopGame(_, _)).Times(0);

    int rid = getNextRequestId();
    router->parseAndAnswer(std::to_string(rid) + " stopGame");

    EXPECT_EQ(responseMessages.size(), 1);
    EXPECT_EQ(responseMessages[0],
              std::to_string(rid) + " Error: stopGame requires session ID and game ID");
}

TEST_F(RouterTest, StopGameWithMissingGameId2)
{
    std::string sessionId = getValidSessionId();
    EXPECT_CALL(*mockGamesController, stopGame(_, _)).Times(0);

    responseMessages.clear();
    int rid = getNextRequestId();
    router->parseAndAnswer(std::to_string(rid) + " stopGame " + sessionId);

    EXPECT_EQ(responseMessages.size(), 1);
    EXPECT_EQ(responseMessages[0],
              std::to_string(rid) + " Error: stopGame requires session ID and game ID");
}

TEST_F(RouterTest, StopGameWithInvalidGameId)
{
    std::string sessionId = getValidSessionId();
    EXPECT_CALL(*mockGamesController, stopGame(_, _)).Times(0);

    responseMessages.clear();
    int rid = getNextRequestId();
    router->parseAndAnswer(std::to_string(rid) + " stopGame " + sessionId + " invalid");

    EXPECT_EQ(responseMessages.size(), 1);
    EXPECT_EQ(responseMessages[0], std::to_string(rid) + " Error: invalid arguments for stopGame");
}

TEST_F(RouterTest, StopGameWithInvalidSessionId)
{
    EXPECT_CALL(*mockGamesController, stopGame(_, _)).Times(0);

    int rid = getNextRequestId();
    router->parseAndAnswer(std::to_string(rid) + " stopGame invalid 5");

    EXPECT_EQ(responseMessages.size(), 1);
    EXPECT_EQ(responseMessages[0], std::to_string(rid) + " Error: Invalid session ID");
}

TEST_F(RouterTest, AddPlayerToGameSuccessfully)
{
    ull userId = 999;
    ull gameId = 3;
    std::string sessionId = getValidSessionId();

    EXPECT_CALL(*mockGamesController, addPlayerToGame(userId, gameId, _)).Times(1);

    responseMessages.clear();
    int rid = getNextRequestId();
    router->parseAndAnswer(std::to_string(rid) + " addPlayerToGame " + sessionId + " 3");

    EXPECT_EQ(responseMessages.size(), 1);
    EXPECT_EQ(responseMessages[0], std::to_string(rid) + " Player added successfully");
}

TEST_F(RouterTest, AddPlayerToGameWithMissingSessionId)
{
    EXPECT_CALL(*mockGamesController, addPlayerToGame(_, _, _)).Times(0);

    int rid = getNextRequestId();
    router->parseAndAnswer(std::to_string(rid) + " addPlayerToGame");

    EXPECT_EQ(responseMessages.size(), 1);
    EXPECT_EQ(responseMessages[0],
              std::to_string(rid) + " Error: addPlayerToGame requires session ID and game ID");
}

TEST_F(RouterTest, AddPlayerToGameWithMissingGameId)
{
    std::string sessionId = getValidSessionId();
    EXPECT_CALL(*mockGamesController, addPlayerToGame(_, _, _)).Times(0);

    responseMessages.clear();
    int rid = getNextRequestId();
    router->parseAndAnswer(std::to_string(rid) + " addPlayerToGame " + sessionId);

    EXPECT_EQ(responseMessages.size(), 1);
    EXPECT_EQ(responseMessages[0],
              std::to_string(rid) + " Error: addPlayerToGame requires session ID and game ID");
}

TEST_F(RouterTest, AddPlayerToGameWithInvalidSessionId)
{
    EXPECT_CALL(*mockGamesController, addPlayerToGame(_, _, _)).Times(0);

    int rid = getNextRequestId();
    router->parseAndAnswer(std::to_string(rid) + " addPlayerToGame invalid 3");

    EXPECT_EQ(responseMessages.size(), 1);
    EXPECT_EQ(responseMessages[0], std::to_string(rid) + " Error: Invalid session ID");
}

TEST_F(RouterTest, HandleWordSuccessfully)
{
    ull gameId = 7;
    ull userId = 999;
    std::string word = "テスト";
    std::string sessionId = getValidSessionId();

    EXPECT_CALL(*mockGamesController, handleWord(gameId, userId, word, _))
        .Times(1)
        .WillOnce(Invoke(
            [](ull gid, ull uid, std::string w, std::function<void(HandleWordStatus)> callback)
            { callback(HandleWordStatus::OK); }));

    responseMessages.clear();
    int rid = getNextRequestId();
    router->parseAndAnswer(std::to_string(rid) + " handleWord 7 " + sessionId +
                           " \u30c6\u30b9\u30c8");

    EXPECT_EQ(responseMessages.size(), 1);
    EXPECT_EQ(responseMessages[0], std::to_string(rid) + " OK");
}

TEST_F(RouterTest, HandleWordWithErrorStatus)
{
    ull gameId = 7;
    ull userId = 999;
    std::string word = "テスト";
    std::string sessionId = getValidSessionId();

    EXPECT_CALL(*mockGamesController, handleWord(gameId, userId, word, _))
        .Times(1)
        .WillOnce(Invoke(
            [](ull gid, ull uid, std::string w, std::function<void(HandleWordStatus)> callback)
            { callback(HandleWordStatus::WRONG_ORDER); }));

    responseMessages.clear();
    int rid = getNextRequestId();
    router->parseAndAnswer(std::to_string(rid) + " handleWord 7 " + sessionId +
                           " \u30c6\u30b9\u30c8");

    EXPECT_EQ(responseMessages.size(), 1);
    EXPECT_EQ(responseMessages[0], std::to_string(rid) + " WRONG_ORDER");
}

TEST_F(RouterTest, HandleWordWithGameNotFoundStatus)
{
    ull gameId = 999;
    ull userId = 999;
    std::string word = "テスト";
    std::string sessionId = getValidSessionId();

    EXPECT_CALL(*mockGamesController, handleWord(gameId, userId, word, _))
        .Times(1)
        .WillOnce(Invoke(
            [](ull gid, ull uid, std::string w, std::function<void(HandleWordStatus)> callback)
            { callback(HandleWordStatus::GAME_NOT_FOUND); }));

    responseMessages.clear();
    int rid = getNextRequestId();
    router->parseAndAnswer(std::to_string(rid) + " handleWord 999 " + sessionId +
                           " \u30c6\u30b9\u30c8");

    EXPECT_EQ(responseMessages.size(), 1);
    EXPECT_EQ(responseMessages[0], std::to_string(rid) + " GAME_NOT_FOUND");
}

TEST_F(RouterTest, HandleWordWithoutGameId)
{
    EXPECT_CALL(*mockGamesController, handleWord(_, _, _, _)).Times(0);

    int rid = getNextRequestId();
    router->parseAndAnswer(std::to_string(rid) + " handleWord");

    EXPECT_EQ(responseMessages.size(), 1);
    EXPECT_EQ(responseMessages[0],
              std::to_string(rid) + " Error: handleWord requires game ID, session ID, and word");
}

TEST_F(RouterTest, HandleWordWithoutSessionId)
{
    EXPECT_CALL(*mockGamesController, handleWord(_, _, _, _)).Times(0);

    int rid = getNextRequestId();
    router->parseAndAnswer(std::to_string(rid) + " handleWord 7");

    EXPECT_EQ(responseMessages.size(), 1);
    EXPECT_EQ(responseMessages[0],
              std::to_string(rid) + " Error: handleWord requires game ID, session ID, and word");
}

TEST_F(RouterTest, HandleWordWithoutWord)
{
    EXPECT_CALL(*mockGamesController, handleWord(_, _, _, _)).Times(0);

    int rid = getNextRequestId();
    router->parseAndAnswer(std::to_string(rid) + " handleWord 7 20");

    EXPECT_EQ(responseMessages.size(), 1);
    EXPECT_EQ(responseMessages[0],
              std::to_string(rid) + " Error: handleWord requires game ID, session ID, and word");
}

TEST_F(RouterTest, HandleWordWithInvalidGameId)
{
    std::string sessionId = getValidSessionId();
    EXPECT_CALL(*mockGamesController, handleWord(_, _, _, _)).Times(0);

    responseMessages.clear();
    int rid = getNextRequestId();
    router->parseAndAnswer(std::to_string(rid) + " handleWord invalid " + sessionId +
                           " \u30c6\u30b9\u30c8");

    EXPECT_EQ(responseMessages.size(), 1);
    EXPECT_EQ(responseMessages[0],
              std::to_string(rid) + " Error: invalid arguments for handleWord");
}

TEST_F(RouterTest, HandleWordWithInvalidSessionId)
{
    EXPECT_CALL(*mockGamesController, handleWord(_, _, _, _)).Times(0);

    int rid = getNextRequestId();
    router->parseAndAnswer(std::to_string(rid) + " handleWord 7 invalid \u30c6\u30b9\u30c8");

    EXPECT_EQ(responseMessages.size(), 1);
    EXPECT_EQ(responseMessages[0], std::to_string(rid) + " Error: Invalid session ID");
}

TEST_F(RouterTest, GetGamesHistorySuccessfully)
{
    ull userId = 999;
    std::vector<GameInfo> history;
    std::string sessionId = getValidSessionId();

    EXPECT_CALL(*mockInfoController, getGamesHistory(userId, _))
        .Times(1)
        .WillOnce(Invoke([](ull id, std::function<void(std::vector<GameInfo>)> callback)
                         { callback({}); }));

    responseMessages.clear();
    int rid = getNextRequestId();
    router->parseAndAnswer(std::to_string(rid) + " getGamesHistory " + sessionId);

    EXPECT_EQ(responseMessages.size(), 1);
}

TEST_F(RouterTest, GetGamesHistoryWithoutSessionId)
{
    EXPECT_CALL(*mockInfoController, getGamesHistory(_, _)).Times(0);

    int rid = getNextRequestId();
    router->parseAndAnswer(std::to_string(rid) + " getGamesHistory");

    EXPECT_EQ(responseMessages.size(), 1);
    EXPECT_EQ(responseMessages[0],
              std::to_string(rid) + " Error: getGamesHistory requires session ID");
}

TEST_F(RouterTest, GetGamesHistoryWithInvalidSessionId)
{
    EXPECT_CALL(*mockInfoController, getGamesHistory(_, _)).Times(0);

    int rid = getNextRequestId();
    router->parseAndAnswer(std::to_string(rid) + " getGamesHistory invalid_session_id");

    EXPECT_EQ(responseMessages.size(), 1);
    EXPECT_EQ(responseMessages[0], std::to_string(rid) + " Error: Invalid session ID");
}

TEST_F(RouterTest, UnknownCommand)
{
    EXPECT_CALL(*mockInfoController, getUserInfo(_, _)).Times(0);
    EXPECT_CALL(*mockInfoController, addUser(_, _)).Times(0);
    EXPECT_CALL(*mockGamesController, getActiveGames(_)).Times(0);

    int rid = getNextRequestId();
    router->parseAndAnswer(std::to_string(rid) + " unknownCommand");

    EXPECT_EQ(responseMessages.size(), 1);
    EXPECT_EQ(responseMessages[0], std::to_string(rid) + " Error: Unknown command");
}

TEST_F(RouterTest, EmptyCommand)
{
    int rid = getNextRequestId();
    router->parseAndAnswer(std::to_string(rid));

    EXPECT_EQ(responseMessages.size(), 1);
    EXPECT_EQ(responseMessages[0], "Error: bad args");
}

TEST_F(RouterTest, StatusToStringOK)
{
    std::string sessionId = getValidSessionId();
    EXPECT_CALL(*mockGamesController, handleWord(_, _, _, _))
        .WillOnce(Invoke([](ull, ull, std::string, std::function<void(HandleWordStatus)> cb)
                         { cb(HandleWordStatus::OK); }));

    responseMessages.clear();
    int rid = getNextRequestId();
    router->parseAndAnswer(std::to_string(rid) + " handleWord 1 " + sessionId + " word");
    EXPECT_EQ(responseMessages[0], std::to_string(rid) + " OK");
}

TEST_F(RouterTest, StatusToStringGotError)
{
    std::string sessionId = getValidSessionId();
    EXPECT_CALL(*mockGamesController, handleWord(_, _, _, _))
        .WillOnce(Invoke([](ull, ull, std::string, std::function<void(HandleWordStatus)> cb)
                         { cb(HandleWordStatus::GOT_ERROR); }));

    responseMessages.clear();
    int rid = getNextRequestId();
    router->parseAndAnswer(std::to_string(rid) + " handleWord 1 " + sessionId + " word");
    EXPECT_EQ(responseMessages[0], std::to_string(rid) + " GOT_ERROR");
}

TEST_F(RouterTest, StatusToStringNotJapaneseWord)
{
    std::string sessionId = getValidSessionId();
    EXPECT_CALL(*mockGamesController, handleWord(_, _, _, _))
        .WillOnce(Invoke([](ull, ull, std::string, std::function<void(HandleWordStatus)> cb)
                         { cb(HandleWordStatus::NOT_JAPANESE_WORD); }));

    responseMessages.clear();
    int rid = getNextRequestId();
    router->parseAndAnswer(std::to_string(rid) + " handleWord 1 " + sessionId + " word");
    EXPECT_EQ(responseMessages[0], std::to_string(rid) + " NOT_JAPANESE_WORD");
}

TEST_F(RouterTest, StatusToStringNoFoundWord)
{
    std::string sessionId = getValidSessionId();
    EXPECT_CALL(*mockGamesController, handleWord(_, _, _, _))
        .WillOnce(Invoke([](ull, ull, std::string, std::function<void(HandleWordStatus)> cb)
                         { cb(HandleWordStatus::NO_FOUND_WORD); }));

    responseMessages.clear();
    int rid = getNextRequestId();
    router->parseAndAnswer(std::to_string(rid) + " handleWord 1 " + sessionId + " word");
    EXPECT_EQ(responseMessages[0], std::to_string(rid) + " NO_FOUND_WORD");
}

TEST_F(RouterTest, StatusToStringGotEndWord)
{
    std::string sessionId = getValidSessionId();
    EXPECT_CALL(*mockGamesController, handleWord(_, _, _, _))
        .WillOnce(Invoke([](ull, ull, std::string, std::function<void(HandleWordStatus)> cb)
                         { cb(HandleWordStatus::GOT_END_WORD); }));

    responseMessages.clear();
    int rid = getNextRequestId();
    router->parseAndAnswer(std::to_string(rid) + " handleWord 1 " + sessionId + " word");
    EXPECT_EQ(responseMessages[0], std::to_string(rid) + " GOT_END_WORD");
}

TEST_F(RouterTest, StatusToStringGameNotFound)
{
    std::string sessionId = getValidSessionId();
    EXPECT_CALL(*mockGamesController, handleWord(_, _, _, _))
        .WillOnce(Invoke([](ull, ull, std::string, std::function<void(HandleWordStatus)> cb)
                         { cb(HandleWordStatus::GAME_NOT_FOUND); }));

    responseMessages.clear();
    int rid = getNextRequestId();
    router->parseAndAnswer(std::to_string(rid) + " handleWord 1 " + sessionId + " word");
    EXPECT_EQ(responseMessages[0], std::to_string(rid) + " GAME_NOT_FOUND");
}

TEST_F(RouterTest, MultipleCommandsInSequence)
{
    std::string sessionId = getValidSessionId();
    ull userId = 999;

    UserInfo user(userId, "TestUser", 5, 10);
    EXPECT_CALL(*mockInfoController, getUserInfo(userId, _))
        .WillOnce(Invoke([user](ull id, std::function<void(UserInfo)> cb) { cb(user); }));

    ull newId = 100;
    EXPECT_CALL(*mockInfoController, addUser("NewUser", _))
        .WillOnce(
            Invoke([newId](const std::string& nick, std::function<void(ull)> cb) { cb(newId); }));

    responseMessages.clear();
    int rid1 = getNextRequestId();
    router->parseAndAnswer(std::to_string(rid1) + " getUserInfo " + sessionId);
    EXPECT_EQ(responseMessages.size(), 1);

    responseMessages.clear();
    int rid2 = getNextRequestId();
    router->parseAndAnswer(std::to_string(rid2) + " addUser NewUser");
    EXPECT_EQ(responseMessages.size(), 1);
    EXPECT_EQ(responseMessages[0], std::to_string(rid2) + " 100");
}
