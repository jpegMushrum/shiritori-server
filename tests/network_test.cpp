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
};

class MockGamesController : public IGamesController
{
  public:
    MOCK_METHOD(void, startNewGame, (ull, std::function<void(GameContext)>), (override));
    MOCK_METHOD(void, handleWord, (ull, ull, std::string, std::function<void(HandleWordStatus)>),
                (override));
    MOCK_METHOD(void, getActiveGames, (std::function<void(std::vector<GameContext>)>), (override));
    MOCK_METHOD(void, addPlayerToGame, (ull, ull), (override));
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
        auto writeCallback = [this](std::string msg) { responseMessages.push_back(msg); };

        router = std::make_unique<Router>(writeCallback, mockInfoController, mockGamesController);
    }

    std::shared_ptr<MockInfoController> mockInfoController;
    std::shared_ptr<MockGamesController> mockGamesController;
    std::unique_ptr<Router> router;
    std::vector<std::string> responseMessages;
};

TEST_F(RouterTest, GetUserSuccessfully)
{
    ull userId = 42;
    UserInfo expectedUser(userId, "Alice", 10, 25);

    EXPECT_CALL(*mockInfoController, getUserInfo(userId, _))
        .Times(1)
        .WillOnce(Invoke([expectedUser](ull id, std::function<void(UserInfo)> callback)
                         { callback(expectedUser); }));

    router->parseAndAnswer("getUserInfo 42");

    EXPECT_EQ(responseMessages.size(), 1);
    EXPECT_NE(responseMessages[0].find("42"), std::string::npos);
    EXPECT_NE(responseMessages[0].find("Alice"), std::string::npos);
}

TEST_F(RouterTest, GetUserWithoutIdArgument)
{
    EXPECT_CALL(*mockInfoController, getUserInfo(_, _)).Times(0);

    router->parseAndAnswer("getUserInfo");

    EXPECT_EQ(responseMessages.size(), 1);
    EXPECT_EQ(responseMessages[0], "Error: getUserInfo requires user ID");
}

TEST_F(RouterTest, GetUserWithInvalidId)
{
    EXPECT_CALL(*mockInfoController, getUserInfo(_, _)).Times(0);

    router->parseAndAnswer("getUserInfo invalid_id");

    EXPECT_EQ(responseMessages.size(), 1);
    EXPECT_EQ(responseMessages[0], "Error: invalid arguments for getUserInfo");
}

TEST_F(RouterTest, AddUserSuccessfully)
{
    std::string nickname = "Bob";
    ull newUserId = 100;

    EXPECT_CALL(*mockInfoController, addUser(nickname, _))
        .Times(1)
        .WillOnce(Invoke([newUserId](const std::string& nick, std::function<void(ull)> callback)
                         { callback(newUserId); }));

    router->parseAndAnswer("addUser Bob");

    EXPECT_EQ(responseMessages.size(), 1);
    EXPECT_EQ(responseMessages[0], "100");
}

TEST_F(RouterTest, AddUserWithoutNickname)
{
    EXPECT_CALL(*mockInfoController, addUser(_, _)).Times(0);

    router->parseAndAnswer("addUser");

    EXPECT_EQ(responseMessages.size(), 1);
    EXPECT_EQ(responseMessages[0], "Error: addUser requires nickname");
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

    router->parseAndAnswer("getActiveGames");

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

    router->parseAndAnswer("getActiveGames");

    EXPECT_EQ(responseMessages.size(), 1);
    EXPECT_EQ(responseMessages[0], "");
}

TEST_F(RouterTest, GetGameInfoSuccessfully)
{
    ull gameId = 42;
    GameContext gameContext(gameId, 10, 2, 50, U'あ');

    EXPECT_CALL(*mockGamesController, getGameInfo(gameId, _))
        .Times(1)
        .WillOnce(Invoke([gameContext](ull id, std::function<void(GameContext)> callback)
                         { callback(gameContext); }));

    router->parseAndAnswer("getGameInfo 42");

    EXPECT_EQ(responseMessages.size(), 1);
    EXPECT_NE(responseMessages[0].find("42"), std::string::npos);
}

TEST_F(RouterTest, GetGameInfoWithoutGameId)
{
    EXPECT_CALL(*mockGamesController, getGameInfo(_, _)).Times(0);

    router->parseAndAnswer("getGameInfo");

    EXPECT_EQ(responseMessages.size(), 1);
    EXPECT_EQ(responseMessages[0], "Error: getGameInfo requires game ID");
}

TEST_F(RouterTest, GetGameInfoWithInvalidGameId)
{
    EXPECT_CALL(*mockGamesController, getGameInfo(_, _)).Times(0);

    router->parseAndAnswer("getGameInfo not_a_number");

    EXPECT_EQ(responseMessages.size(), 1);
    EXPECT_EQ(responseMessages[0], "Error: invalid arguments for getGameInfo");
}

TEST_F(RouterTest, StartNewGameSuccessfully)
{
    ull userId = 10;
    GameContext newGame(1, 0, 1, userId, U'あ');

    EXPECT_CALL(*mockGamesController, startNewGame(userId, _))
        .Times(1)
        .WillOnce(Invoke([newGame](ull id, std::function<void(GameContext)> callback)
                         { callback(newGame); }));

    router->parseAndAnswer("startNewGame 10");

    EXPECT_EQ(responseMessages.size(), 1);
    EXPECT_NE(responseMessages[0].find("1"), std::string::npos);
}

TEST_F(RouterTest, StartNewGameWithoutUserId)
{
    EXPECT_CALL(*mockGamesController, startNewGame(_, _)).Times(0);

    router->parseAndAnswer("startNewGame");

    EXPECT_EQ(responseMessages.size(), 1);
    EXPECT_EQ(responseMessages[0], "Error: startNewGame requires user ID");
}

TEST_F(RouterTest, StartNewGameWithInvalidUserId)
{
    EXPECT_CALL(*mockGamesController, startNewGame(_, _)).Times(0);

    router->parseAndAnswer("startNewGame abc");

    EXPECT_EQ(responseMessages.size(), 1);
    EXPECT_EQ(responseMessages[0], "Error: invalid arguments for startNewGame");
}

TEST_F(RouterTest, StopGameSuccessfully)
{
    ull gameId = 5;
    ull userId = 10;

    EXPECT_CALL(*mockGamesController, stopGame(gameId, userId)).Times(1);

    router->parseAndAnswer("stopGame 5 10");

    EXPECT_EQ(responseMessages.size(), 1);
    EXPECT_EQ(responseMessages[0], "Game stopped successfully");
}

TEST_F(RouterTest, StopGameWithMissingGameId)
{
    EXPECT_CALL(*mockGamesController, stopGame(_, _)).Times(0);

    router->parseAndAnswer("stopGame");

    EXPECT_EQ(responseMessages.size(), 1);
    EXPECT_EQ(responseMessages[0], "Error: stopGame requires game ID and user ID");
}

TEST_F(RouterTest, StopGameWithMissingUserId)
{
    EXPECT_CALL(*mockGamesController, stopGame(_, _)).Times(0);

    router->parseAndAnswer("stopGame 5");

    EXPECT_EQ(responseMessages.size(), 1);
    EXPECT_EQ(responseMessages[0], "Error: stopGame requires game ID and user ID");
}

TEST_F(RouterTest, StopGameWithInvalidGameId)
{
    EXPECT_CALL(*mockGamesController, stopGame(_, _)).Times(0);

    router->parseAndAnswer("stopGame invalid 10");

    EXPECT_EQ(responseMessages.size(), 1);
    EXPECT_EQ(responseMessages[0], "Error: invalid arguments for stopGame");
}

TEST_F(RouterTest, StopGameWithInvalidUserId)
{
    EXPECT_CALL(*mockGamesController, stopGame(_, _)).Times(0);

    router->parseAndAnswer("stopGame 5 invalid");

    EXPECT_EQ(responseMessages.size(), 1);
    EXPECT_EQ(responseMessages[0], "Error: invalid arguments for stopGame");
}

TEST_F(RouterTest, AddPlayerToGameSuccessfully)
{
    ull userId = 15;
    ull gameId = 3;

    EXPECT_CALL(*mockGamesController, addPlayerToGame(userId, gameId)).Times(1);

    router->parseAndAnswer("addPlayerToGame 15 3");

    EXPECT_EQ(responseMessages.size(), 1);
    EXPECT_EQ(responseMessages[0], "Player added successfully");
}

TEST_F(RouterTest, AddPlayerToGameWithMissingUserId)
{
    EXPECT_CALL(*mockGamesController, addPlayerToGame(_, _)).Times(0);

    router->parseAndAnswer("addPlayerToGame");

    EXPECT_EQ(responseMessages.size(), 1);
    EXPECT_EQ(responseMessages[0], "Error: addPlayerToGame requires user ID and game ID");
}

TEST_F(RouterTest, AddPlayerToGameWithMissingGameId)
{
    EXPECT_CALL(*mockGamesController, addPlayerToGame(_, _)).Times(0);

    router->parseAndAnswer("addPlayerToGame 15");

    EXPECT_EQ(responseMessages.size(), 1);
    EXPECT_EQ(responseMessages[0], "Error: addPlayerToGame requires user ID and game ID");
}

TEST_F(RouterTest, AddPlayerToGameWithInvalidUserId)
{
    EXPECT_CALL(*mockGamesController, addPlayerToGame(_, _)).Times(0);

    router->parseAndAnswer("addPlayerToGame invalid 3");

    EXPECT_EQ(responseMessages.size(), 1);
    EXPECT_EQ(responseMessages[0], "Error: invalid arguments for addPlayerToGame");
}

TEST_F(RouterTest, HandleWordSuccessfully)
{
    ull gameId = 7;
    ull userId = 20;
    std::string word = "テスト";

    EXPECT_CALL(*mockGamesController, handleWord(gameId, userId, word, _))
        .Times(1)
        .WillOnce(Invoke(
            [](ull gid, ull uid, std::string w, std::function<void(HandleWordStatus)> callback)
            { callback(HandleWordStatus::OK); }));

    router->parseAndAnswer("handleWord 7 20 テスト");

    EXPECT_EQ(responseMessages.size(), 1);
    EXPECT_EQ(responseMessages[0], "OK");
}

TEST_F(RouterTest, HandleWordWithErrorStatus)
{
    ull gameId = 7;
    ull userId = 20;
    std::string word = "テスト";

    EXPECT_CALL(*mockGamesController, handleWord(gameId, userId, word, _))
        .Times(1)
        .WillOnce(Invoke(
            [](ull gid, ull uid, std::string w, std::function<void(HandleWordStatus)> callback)
            { callback(HandleWordStatus::WRONG_ORDER); }));

    router->parseAndAnswer("handleWord 7 20 テスト");

    EXPECT_EQ(responseMessages.size(), 1);
    EXPECT_EQ(responseMessages[0], "WRONG_ORDER");
}

TEST_F(RouterTest, HandleWordWithGameNotFoundStatus)
{
    ull gameId = 999;
    ull userId = 20;
    std::string word = "テスト";

    EXPECT_CALL(*mockGamesController, handleWord(gameId, userId, word, _))
        .Times(1)
        .WillOnce(Invoke(
            [](ull gid, ull uid, std::string w, std::function<void(HandleWordStatus)> callback)
            { callback(HandleWordStatus::GAME_NOT_FOUND); }));

    router->parseAndAnswer("handleWord 999 20 テスト");

    EXPECT_EQ(responseMessages.size(), 1);
    EXPECT_EQ(responseMessages[0], "GAME_NOT_FOUND");
}

TEST_F(RouterTest, HandleWordWithoutGameId)
{
    EXPECT_CALL(*mockGamesController, handleWord(_, _, _, _)).Times(0);

    router->parseAndAnswer("handleWord");

    EXPECT_EQ(responseMessages.size(), 1);
    EXPECT_EQ(responseMessages[0], "Error: handleWord requires game ID, user ID, and word");
}

TEST_F(RouterTest, HandleWordWithoutUserId)
{
    EXPECT_CALL(*mockGamesController, handleWord(_, _, _, _)).Times(0);

    router->parseAndAnswer("handleWord 7");

    EXPECT_EQ(responseMessages.size(), 1);
    EXPECT_EQ(responseMessages[0], "Error: handleWord requires game ID, user ID, and word");
}

TEST_F(RouterTest, HandleWordWithoutWord)
{
    EXPECT_CALL(*mockGamesController, handleWord(_, _, _, _)).Times(0);

    router->parseAndAnswer("handleWord 7 20");

    EXPECT_EQ(responseMessages.size(), 1);
    EXPECT_EQ(responseMessages[0], "Error: handleWord requires game ID, user ID, and word");
}

TEST_F(RouterTest, HandleWordWithInvalidGameId)
{
    EXPECT_CALL(*mockGamesController, handleWord(_, _, _, _)).Times(0);

    router->parseAndAnswer("handleWord invalid 20 テスト");

    EXPECT_EQ(responseMessages.size(), 1);
    EXPECT_EQ(responseMessages[0], "Error: invalid arguments for handleWord");
}

TEST_F(RouterTest, HandleWordWithInvalidUserId)
{
    EXPECT_CALL(*mockGamesController, handleWord(_, _, _, _)).Times(0);

    router->parseAndAnswer("handleWord 7 invalid テスト");

    EXPECT_EQ(responseMessages.size(), 1);
    EXPECT_EQ(responseMessages[0], "Error: invalid arguments for handleWord");
}

TEST_F(RouterTest, GetGamesHistorySuccessfully)
{
    ull userId = 25;
    std::vector<GameInfo> history;

    EXPECT_CALL(*mockInfoController, getGamesHistory(userId, _))
        .Times(1)
        .WillOnce(Invoke([](ull id, std::function<void(std::vector<GameInfo>)> callback)
                         { callback({}); }));

    router->parseAndAnswer("getGamesHistory 25");

    EXPECT_EQ(responseMessages.size(), 1);
}

TEST_F(RouterTest, GetGamesHistoryWithoutUserId)
{
    EXPECT_CALL(*mockInfoController, getGamesHistory(_, _)).Times(0);

    router->parseAndAnswer("getGamesHistory");

    EXPECT_EQ(responseMessages.size(), 1);
    EXPECT_EQ(responseMessages[0], "Error: handleWord requires game ID, user ID, and word");
}

TEST_F(RouterTest, GetGamesHistoryWithInvalidUserId)
{
    EXPECT_CALL(*mockInfoController, getGamesHistory(_, _)).Times(0);

    router->parseAndAnswer("getGamesHistory not_a_number");

    EXPECT_EQ(responseMessages.size(), 1);
    EXPECT_EQ(responseMessages[0], "Error: invalid arguments for handleWord");
}

TEST_F(RouterTest, UnknownCommand)
{
    EXPECT_CALL(*mockInfoController, getUserInfo(_, _)).Times(0);
    EXPECT_CALL(*mockInfoController, addUser(_, _)).Times(0);
    EXPECT_CALL(*mockGamesController, getActiveGames(_)).Times(0);

    router->parseAndAnswer("unknownCommand");

    EXPECT_EQ(responseMessages.size(), 1);
    EXPECT_EQ(responseMessages[0], "Error: Unknown command");
}

TEST_F(RouterTest, EmptyCommand)
{
    router->parseAndAnswer("");

    EXPECT_EQ(responseMessages.size(), 1);
    EXPECT_EQ(responseMessages[0], "Error: Unknown command");
}

TEST_F(RouterTest, StatusToStringOK)
{
    EXPECT_CALL(*mockGamesController, handleWord(_, _, _, _))
        .WillOnce(Invoke([](ull, ull, std::string, std::function<void(HandleWordStatus)> cb)
                         { cb(HandleWordStatus::OK); }));

    router->parseAndAnswer("handleWord 1 1 word");
    EXPECT_EQ(responseMessages[0], "OK");
}

TEST_F(RouterTest, StatusToStringGotError)
{
    EXPECT_CALL(*mockGamesController, handleWord(_, _, _, _))
        .WillOnce(Invoke([](ull, ull, std::string, std::function<void(HandleWordStatus)> cb)
                         { cb(HandleWordStatus::GOT_ERROR); }));

    router->parseAndAnswer("handleWord 1 1 word");
    EXPECT_EQ(responseMessages[0], "GOT_ERROR");
}

TEST_F(RouterTest, StatusToStringNotJapaneseWord)
{
    EXPECT_CALL(*mockGamesController, handleWord(_, _, _, _))
        .WillOnce(Invoke([](ull, ull, std::string, std::function<void(HandleWordStatus)> cb)
                         { cb(HandleWordStatus::NOT_JAPANESE_WORD); }));

    router->parseAndAnswer("handleWord 1 1 word");
    EXPECT_EQ(responseMessages[0], "NOT_JAPANESE_WORD");
}

TEST_F(RouterTest, StatusToStringNoFoundWord)
{
    EXPECT_CALL(*mockGamesController, handleWord(_, _, _, _))
        .WillOnce(Invoke([](ull, ull, std::string, std::function<void(HandleWordStatus)> cb)
                         { cb(HandleWordStatus::NO_FOUND_WORD); }));

    router->parseAndAnswer("handleWord 1 1 word");
    EXPECT_EQ(responseMessages[0], "NO_FOUND_WORD");
}

TEST_F(RouterTest, StatusToStringGotEndWord)
{
    EXPECT_CALL(*mockGamesController, handleWord(_, _, _, _))
        .WillOnce(Invoke([](ull, ull, std::string, std::function<void(HandleWordStatus)> cb)
                         { cb(HandleWordStatus::GOT_END_WORD); }));

    router->parseAndAnswer("handleWord 1 1 word");
    EXPECT_EQ(responseMessages[0], "GOT_END_WORD");
}

TEST_F(RouterTest, StatusToStringGameNotFound)
{
    EXPECT_CALL(*mockGamesController, handleWord(_, _, _, _))
        .WillOnce(Invoke([](ull, ull, std::string, std::function<void(HandleWordStatus)> cb)
                         { cb(HandleWordStatus::GAME_NOT_FOUND); }));

    router->parseAndAnswer("handleWord 1 1 word");
    EXPECT_EQ(responseMessages[0], "GAME_NOT_FOUND");
}

TEST_F(RouterTest, MultipleCommandsInSequence)
{
    UserInfo user(1, "TestUser", 5, 10);
    EXPECT_CALL(*mockInfoController, getUserInfo(1, _))
        .WillOnce(Invoke([user](ull id, std::function<void(UserInfo)> cb) { cb(user); }));

    ull newId = 100;
    EXPECT_CALL(*mockInfoController, addUser("NewUser", _))
        .WillOnce(
            Invoke([newId](const std::string& nick, std::function<void(ull)> cb) { cb(newId); }));

    router->parseAndAnswer("getUserInfo 1");
    router->parseAndAnswer("addUser NewUser");

    EXPECT_EQ(responseMessages.size(), 2);
    EXPECT_EQ(responseMessages[1], "100");
}
