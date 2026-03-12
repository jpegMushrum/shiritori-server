#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <future>

#include "iinfo_service.hpp"
#include "info_controller.hpp"
#include "task_queue.hpp"
#include "user_info.hpp"

#include "game_context.hpp"
#include "games_controller.hpp"
#include "igame_fabric.hpp"
#include "igame_session.hpp"

using ::testing::_;
using ::testing::Invoke;
using ::testing::Return;

using ull = unsigned long long;

class MockInfoService : public IInfoService
{
  public:
    MOCK_METHOD(UserInfo, getUserInfo, (ull), (override));
    MOCK_METHOD(ull, addUser, (const std::string&), (override));
};

class InfoControllerTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        taskQueue = std::make_shared<TaskQueue>(1);

        infoService = new MockInfoService();

        controller =
            std::make_unique<InfoController>(taskQueue, std::unique_ptr<IInfoService>(infoService));
    }

    std::shared_ptr<TaskQueue> taskQueue;
    MockInfoService* infoService;
    std::unique_ptr<InfoController> controller;
};

TEST_F(InfoControllerTest, GetUserInfoCallsServiceAndReturnsResult)
{
    UserInfo expected(42, "a", 42, 42);

    EXPECT_CALL(*infoService, getUserInfo(42)).Times(1).WillOnce(Return(expected));

    std::promise<UserInfo> promise;
    auto future = promise.get_future();

    controller->getUserInfo(42, [&](UserInfo info) { promise.set_value(info); });

    UserInfo result = future.get();

    EXPECT_EQ(result.id, 42);
    EXPECT_EQ(result.nickname, "a");
    EXPECT_EQ(result.words, 42);
    EXPECT_EQ(result.games, 42);
}

TEST_F(InfoControllerTest, AddUserCallsServiceAndReturnsId)
{
    EXPECT_CALL(*infoService, addUser("Alice")).Times(1).WillOnce(Return(100));

    std::promise<ull> promise;
    auto future = promise.get_future();

    controller->addUser("Alice", [&](ull id) { promise.set_value(id); });

    ull result = future.get();

    EXPECT_EQ(result, 100);
}

class MockGameSession : public IGameSession
{
  public:
    MOCK_METHOD(void, addUser, (ull), (override));
    MOCK_METHOD(HandleWordStatus, handleWord, (ull, const std::string&), (override));
    MOCK_METHOD(void, stopGame, (), (override));
    MOCK_METHOD(GameContext, getInfo, (), (override));
};

class MockGameFabric : public IGameFabric
{
  public:
    MOCK_METHOD(std::shared_ptr<IGameSession>, createGame, (ull, ull), (override));
};

class GamesControllerTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        taskQueue = std::make_shared<TaskQueue>(1);

        mockFabric = new MockGameFabric();

        controller =
            std::make_unique<GamesController>(taskQueue, std::unique_ptr<IGameFabric>(mockFabric));
    }

    std::shared_ptr<TaskQueue> taskQueue;
    MockGameFabric* mockFabric;
    std::unique_ptr<GamesController> controller;
};

TEST_F(GamesControllerTest, StartNewGameCreatesSessionAndReturnsContext)
{
    ull adminId = 1;
    ull gameId = 42;

    auto mockSession = std::make_shared<MockGameSession>();
    GameContext expectedContext(gameId, 0, 0, adminId, U'あ');

    EXPECT_CALL(*mockFabric, createGame(_, _)).WillOnce(Return(mockSession));

    EXPECT_CALL(*mockSession, getInfo()).WillOnce(Return(expectedContext));

    std::promise<GameContext> promise;
    auto future = promise.get_future();

    controller->startNewGame(adminId, [&](GameContext ctx) { promise.set_value(ctx); });

    GameContext result = future.get();
    EXPECT_EQ(result.id, gameId);
    EXPECT_EQ(result.adminId, adminId);
}

TEST_F(GamesControllerTest, HandleWordCallsSessionAndReturnsStatus)
{
    ull gameId = 0;
    ull playerId = 100;

    auto mockSession = std::make_shared<MockGameSession>();
    EXPECT_CALL(*mockFabric, createGame(_, _)).WillOnce(Return(mockSession));
    EXPECT_CALL(*mockSession, getInfo()).WillOnce(Return(GameContext(0, 0, 0, playerId, U'あ')));

    controller->startNewGame(playerId, [](GameContext) {});

    EXPECT_CALL(*mockSession, handleWord(playerId, "テスト"))
        .WillOnce(Return(HandleWordStatus::OK));

    std::promise<HandleWordStatus> promise;
    auto future = promise.get_future();

    controller->handleWord(gameId, playerId, "テスト",
                           [&](HandleWordStatus status) { promise.set_value(status); });

    EXPECT_EQ(future.get(), HandleWordStatus::OK);
}

TEST_F(GamesControllerTest, GetActiveGamesReturnsAllContexts)
{
    auto mockSession1 = std::make_shared<MockGameSession>();
    auto mockSession2 = std::make_shared<MockGameSession>();

    ull game1 = 0, admin1 = 10;
    ull game2 = 1, admin2 = 20;

    GameContext ctx1(game1, 0, 0, admin1, U'あ');
    GameContext ctx2(game2, 0, 0, admin2, U'う');

    EXPECT_CALL(*mockFabric, createGame(game1, admin1)).WillOnce(Return(mockSession1));
    EXPECT_CALL(*mockSession1, getInfo()).WillRepeatedly(Return(ctx1));
    controller->startNewGame(admin1, [](GameContext) {});

    EXPECT_CALL(*mockFabric, createGame(game2, admin2)).WillOnce(Return(mockSession2));
    EXPECT_CALL(*mockSession2, getInfo()).WillRepeatedly(Return(ctx2));
    controller->startNewGame(admin2, [](GameContext) {});

    std::promise<std::vector<GameContext>> promise;
    auto future = promise.get_future();

    controller->getActiveGames([&](std::vector<GameContext> ctxs) { promise.set_value(ctxs); });

    auto result = future.get();
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0].id, game1);
    EXPECT_EQ(result[1].id, game2);
    EXPECT_EQ(result[0].adminId, admin1);
    EXPECT_EQ(result[1].adminId, admin2);
}

TEST_F(GamesControllerTest, AddPlayerToGameCallsAddUser)
{
    ull gameId = 0;
    ull playerId = 100;

    GameContext ctx1(gameId, 0, 0, playerId, U'あ');
    auto mockSession = std::make_shared<MockGameSession>();

    EXPECT_CALL(*mockFabric, createGame(gameId, playerId)).WillOnce(Return(mockSession));
    EXPECT_CALL(*mockSession, getInfo()).WillRepeatedly(Return(ctx1));
    controller->startNewGame(playerId, [](GameContext) {});

    EXPECT_CALL(*mockSession, addUser(playerId)).Times(1);
    controller->addPlayerToGame(playerId, gameId);

    // Waiting
    std::promise<GameContext> promise;
    auto future = promise.get_future();
    controller->getGameInfo(gameId, [&](GameContext ctx) { promise.set_value(ctx); });
    future.get();
}

TEST_F(GamesControllerTest, StopGameCallsSessionStop)
{
    ull gameId = 0;
    ull playerId = 100;

    GameContext ctx1(gameId, 0, 0, playerId, U'あ');
    auto mockSession = std::make_shared<MockGameSession>();

    EXPECT_CALL(*mockFabric, createGame(gameId, playerId)).WillOnce(Return(mockSession));
    EXPECT_CALL(*mockSession, getInfo()).WillRepeatedly(Return(ctx1));
    controller->startNewGame(playerId, [](GameContext) {});

    EXPECT_CALL(*mockSession, stopGame()).Times(1);
    controller->stopGame(gameId, playerId);

    // Waiting
    std::promise<std::vector<GameContext>> promise;
    auto future = promise.get_future();
    controller->getActiveGames([&](std::vector<GameContext> ctxs) { promise.set_value(ctxs); });
    future.get();
}

TEST_F(GamesControllerTest, GetGameInfoReturnsCorrectContext)
{
    ull gameId = 0;
    ull playerId = 100;

    GameContext ctx(gameId, 1, 1, playerId, U'あ');
    auto mockSession = std::make_shared<MockGameSession>();

    EXPECT_CALL(*mockFabric, createGame(gameId, playerId)).WillOnce(Return(mockSession));
    EXPECT_CALL(*mockSession, getInfo()).WillRepeatedly(Return(ctx));
    controller->startNewGame(playerId, [](GameContext) {});

    std::promise<GameContext> promise;
    auto future = promise.get_future();

    controller->getGameInfo(gameId, [&](GameContext c) { promise.set_value(c); });

    auto result = future.get();
    EXPECT_EQ(result.id, gameId);
    EXPECT_EQ(result.adminId, playerId);
    EXPECT_EQ(result.wordsCount, 1);
    EXPECT_EQ(result.playersCount, 1);
    EXPECT_EQ(result.lastKana, U'あ');
}