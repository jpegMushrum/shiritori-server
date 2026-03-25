#include <gmock/gmock-actions.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <vector>

#include "idictionary.hpp"
#include "igames_repo.hpp"
#include "iusers_repo.hpp"

#include "game_fabric.hpp"
#include "game_session.hpp"
#include "info_service.hpp"

using ::testing::_;
using ::testing::Return;

using ull = unsigned long long;

class MockDictionary : public IDictionary
{
  public:
    MOCK_METHOD(std::vector<Word>, searchWord, (const std::string&), (override));
};

class MockGamesRepo : public IGamesRepo
{
  public:
    MOCK_METHOD(void, putGame, (std::vector<Game>), (override));
    MOCK_METHOD(std::vector<Game>, getGameHistoryByUserId, (ull), (override));
};

class MockUsersRepo : public IUsersRepo
{
  public:
    MOCK_METHOD(User, getUser, (ull), (override));
    MOCK_METHOD(ull, addUser, (User), (override));
    MOCK_METHOD(void, changeUser, (User), (override));
};

class GameSessionTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        dict = std::make_shared<MockDictionary>();
        repo = std::make_shared<MockGamesRepo>();

        session = std::make_unique<GameSession>(1, 10, dict, repo, U'ね');
        EXPECT_CALL(*repo, putGame(_)).WillOnce(Return());
    }

    std::shared_ptr<MockDictionary> dict;
    std::shared_ptr<MockGamesRepo> repo;
    std::unique_ptr<GameSession> session;
};

TEST_F(GameSessionTest, AddUserAddsPlayer)
{
    session->addUser(42);

    auto ctx = session->getInfo();

    EXPECT_EQ(ctx.playersCount, 1);
}

TEST_F(GameSessionTest, HandleWordValidWord)
{
    session->addUser(1);

    Word word;
    word.partsOfSpeach.insert("Noun");
    word.readings.emplace_back("ねこ");

    std::vector<Word> dictResult{word};

    EXPECT_CALL(*dict, searchWord("ねこ")).WillOnce(Return(dictResult));

    auto status = session->handleWord(1, "ねこ");

    EXPECT_EQ(status, HandleWordStatus::OK);
}

TEST_F(GameSessionTest, HandleWordWrongOrder)
{
    session->addUser(0);

    auto status = session->handleWord(0, "ねこ");

    EXPECT_EQ(status, HandleWordStatus::WRONG_ORDER);
}

TEST_F(GameSessionTest, HandleWordNotJapaneseWord)
{
    session->addUser(1);

    auto status = session->handleWord(1, "english");

    EXPECT_EQ(status, HandleWordStatus::NOT_JAPANESE_WORD);
}

TEST_F(GameSessionTest, HandleWordNoSpeachPart)
{
    session->addUser(1);

    Word word;
    word.partsOfSpeach.insert("Verb");
    word.readings.emplace_back("ねる");

    std::vector<Word> dictResult{word};

    EXPECT_CALL(*dict, searchWord("ねる")).WillOnce(Return(dictResult));

    auto status = session->handleWord(1, "ねる");

    EXPECT_EQ(status, HandleWordStatus::NO_SPEACH_PART);
}

TEST_F(GameSessionTest, HandleWordNoFoundWord)
{
    session->addUser(1);

    std::vector<Word> dictResult;

    EXPECT_CALL(*dict, searchWord("ねね")).WillOnce(Return(dictResult));

    auto status = session->handleWord(1, "ねね");

    EXPECT_EQ(status, HandleWordStatus::NO_FOUND_WORD);
}

TEST_F(GameSessionTest, HandleWordGotEndWord)
{
    session->addUser(1);

    Word word;
    word.partsOfSpeach.insert("Noun");
    word.readings.emplace_back("ねん");

    std::vector<Word> dictResult{word};

    EXPECT_CALL(*dict, searchWord("ねん")).WillOnce(Return(dictResult));

    auto status = session->handleWord(1, "ねん");

    EXPECT_EQ(status, HandleWordStatus::GOT_END_WORD);
}

TEST_F(GameSessionTest, HandleWordGotDoubledWord)
{
    session->addUser(1);

    Word word;
    word.kanji = "ねこ";
    word.partsOfSpeach.insert("Noun");
    word.readings.emplace_back("ねこ");

    std::vector<Word> dictResult{word};

    EXPECT_CALL(*dict, searchWord("ねこ")).WillOnce(Return(dictResult));

    auto status1 = session->handleWord(1, "ねこ");
    EXPECT_EQ(status1, HandleWordStatus::OK);

    session->addUser(2);
    auto status2 = session->handleWord(2, "ねこ");

    EXPECT_EQ(status2, HandleWordStatus::GOT_DOUBLED_WORD);
}

TEST_F(GameSessionTest, HandleWordCantJoinWords)
{
    session->addUser(1);

    Word word;
    word.kanji = "ねこ";
    word.partsOfSpeach.insert("Noun");
    word.readings.emplace_back("ねこ");

    std::vector<Word> dictResult{word};

    EXPECT_CALL(*dict, searchWord("ねこ")).WillOnce(Return(dictResult));

    auto status = session->handleWord(1, "ねこ");

    session->addUser(2);

    Word wrongWord;
    wrongWord.kanji = "あかい";
    wrongWord.partsOfSpeach.insert("Noun");
    wrongWord.readings.emplace_back("あかい");

    std::vector<Word> dictResult2{wrongWord};

    EXPECT_CALL(*dict, searchWord("あかい")).WillOnce(Return(dictResult2));

    auto status2 = session->handleWord(2, "あかい");

    EXPECT_EQ(status2, HandleWordStatus::CANT_JOIN_WORDS);
}

TEST_F(GameSessionTest, HandleWordGameStopped)
{
    session->addUser(1);

    session->stopGame();

    auto status = session->handleWord(1, "ねこ");

    EXPECT_EQ(status, HandleWordStatus::GAME_STOPPED);
}

TEST_F(GameSessionTest, HandleWordNoFoundPlayer)
{
    session->addUser(1);

    auto status = session->handleWord(999, "ねこ");

    EXPECT_EQ(status, HandleWordStatus::NO_FOUND_PLAYER);
}

TEST_F(GameSessionTest, HandleWordSuccessfulSequence)
{
    session->addUser(1);
    session->addUser(2);

    Word word1;
    word1.kanji = "ねこ";
    word1.partsOfSpeach.insert("Noun");
    word1.readings.emplace_back("ねこ");

    std::vector<Word> dictResult1{word1};

    EXPECT_CALL(*dict, searchWord("ねこ")).WillOnce(Return(dictResult1));

    auto status1 = session->handleWord(1, "ねこ");
    EXPECT_EQ(status1, HandleWordStatus::OK);

    Word word2;
    word2.kanji = "公園";
    word2.partsOfSpeach.insert("Noun");
    word2.readings.emplace_back("こうえん");

    std::vector<Word> dictResult2{word2};

    EXPECT_CALL(*dict, searchWord("こうえん")).WillOnce(Return(dictResult2));

    auto status2 = session->handleWord(2, "こうえん");
    EXPECT_EQ(status2, HandleWordStatus::GOT_END_WORD);

    auto ctx = session->getInfo();
    EXPECT_EQ(ctx.wordsCount, 1);
    EXPECT_EQ(ctx.playersCount, 2);
}

class GameFabricTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        dict = std::make_shared<MockDictionary>();
        repo = std::make_shared<MockGamesRepo>();
        fabric = std::make_unique<GameFabric>(dict, repo);

        EXPECT_CALL(*repo, putGame(_)).WillRepeatedly(Return());
    }

    std::shared_ptr<MockDictionary> dict;
    std::shared_ptr<MockGamesRepo> repo;
    std::unique_ptr<GameFabric> fabric;
};

TEST_F(GameFabricTest, CreateGameReturnsValidGameSession)
{
    auto game = fabric->createGame(1, 42);

    EXPECT_NE(game, nullptr);

    auto ctx = game->getInfo();
    EXPECT_EQ(ctx.id, 1);
    EXPECT_EQ(ctx.adminId, 42);
    EXPECT_EQ(ctx.playersCount, 0);
    EXPECT_EQ(ctx.wordsCount, 0);
}

TEST_F(GameFabricTest, CreateMultipleGamesWithDifferentIds)
{
    auto game1 = fabric->createGame(1, 42);
    auto game2 = fabric->createGame(2, 43);

    auto ctx1 = game1->getInfo();
    auto ctx2 = game2->getInfo();

    EXPECT_EQ(ctx1.id, 1);
    EXPECT_EQ(ctx2.id, 2);
    EXPECT_EQ(ctx1.adminId, 42);
    EXPECT_EQ(ctx2.adminId, 43);
}

TEST_F(GameFabricTest, CreateGameWithDifferentAdmins)
{
    auto game1 = fabric->createGame(100, 1);
    auto game2 = fabric->createGame(100, 2);

    auto ctx1 = game1->getInfo();
    auto ctx2 = game2->getInfo();

    EXPECT_EQ(ctx1.adminId, 1);
    EXPECT_EQ(ctx2.adminId, 2);
}

class InfoServiceTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        usersRepo = std::make_shared<MockUsersRepo>();
        gamesRepo = std::make_shared<MockGamesRepo>();
        service = std::make_unique<InfoService>(usersRepo, gamesRepo);
    }

    std::shared_ptr<MockUsersRepo> usersRepo;
    std::shared_ptr<MockGamesRepo> gamesRepo;
    std::unique_ptr<InfoService> service;
};

TEST_F(InfoServiceTest, GetUserInfoReturnsUserData)
{
    User expectedUser(42, "Alice");

    EXPECT_CALL(*usersRepo, getUser(42)).WillOnce(Return(expectedUser));

    auto userInfo = service->getUserInfo(42);

    EXPECT_EQ(userInfo.id, 42);
    EXPECT_EQ(userInfo.nickname, "Alice");
}

TEST_F(InfoServiceTest, GetUserInfoMultipleCalls)
{
    User user1(1, "Bob");
    User user2(2, "Charlie");

    EXPECT_CALL(*usersRepo, getUser(1)).WillOnce(Return(user1));
    EXPECT_CALL(*usersRepo, getUser(2)).WillOnce(Return(user2));

    auto info1 = service->getUserInfo(1);
    auto info2 = service->getUserInfo(2);

    EXPECT_EQ(info1.nickname, "Bob");
    EXPECT_EQ(info2.nickname, "Charlie");
}

TEST_F(InfoServiceTest, AddUserCreatesNewUser)
{
    EXPECT_CALL(*usersRepo, addUser(_)).WillOnce(Return(123));

    ull newUserId = service->addUser("NewUser");

    EXPECT_EQ(newUserId, 123);
}

TEST_F(InfoServiceTest, AddUserWithDifferentNicknames)
{
    EXPECT_CALL(*usersRepo, addUser(_))
        .Times(3)
        .WillOnce(Return(1))
        .WillOnce(Return(2))
        .WillOnce(Return(3));

    ull id1 = service->addUser("User1");
    ull id2 = service->addUser("User2");
    ull id3 = service->addUser("User3");

    EXPECT_EQ(id1, 1);
    EXPECT_EQ(id2, 2);
    EXPECT_EQ(id3, 3);
}

TEST_F(InfoServiceTest, AddUserWithEmptyNickname)
{
    EXPECT_CALL(*usersRepo, addUser(_)).WillOnce(Return(999));

    ull userId = service->addUser("");

    EXPECT_EQ(userId, 999);
}

TEST_F(InfoServiceTest, AddUserWithLongNickname)
{
    std::string longNickname(100, 'a');

    EXPECT_CALL(*usersRepo, addUser(_)).WillOnce(Return(555));

    ull userId = service->addUser(longNickname);

    EXPECT_EQ(userId, 555);
}

TEST_F(InfoServiceTest, GetUserInfoAfterAddUser)
{
    User newUser(0, "TestUser");

    EXPECT_CALL(*usersRepo, addUser(_)).WillOnce(Return(888));
    EXPECT_CALL(*usersRepo, getUser(888)).WillOnce(Return(User(888, "TestUser")));

    ull userId = service->addUser("TestUser");
    auto userInfo = service->getUserInfo(userId);

    EXPECT_EQ(userInfo.id, 888);
    EXPECT_EQ(userInfo.nickname, "TestUser");
}

TEST_F(InfoServiceTest, GetUserInfoCalculatesGamesAndWordsCorrectly)
{
    User user(42, "Player");
    std::vector<Game> gameHistory{Game(1, 42, 10, 1), Game(2, 42, 15, 2), Game(3, 42, 5, 3)};

    EXPECT_CALL(*usersRepo, getUser(42)).WillOnce(Return(user));
    EXPECT_CALL(*gamesRepo, getGameHistoryByUserId(42)).WillOnce(Return(gameHistory));

    auto userInfo = service->getUserInfo(42);

    EXPECT_EQ(userInfo.id, 42);
    EXPECT_EQ(userInfo.nickname, "Player");
    EXPECT_EQ(userInfo.games, 3);
    EXPECT_EQ(userInfo.words, 30);
}

TEST_F(InfoServiceTest, GetUserInfoWithEmptyGameHistory)
{
    User user(100, "NewPlayer");
    std::vector<Game> emptyHistory;

    EXPECT_CALL(*usersRepo, getUser(100)).WillOnce(Return(user));
    EXPECT_CALL(*gamesRepo, getGameHistoryByUserId(100)).WillOnce(Return(emptyHistory));

    auto userInfo = service->getUserInfo(100);

    EXPECT_EQ(userInfo.id, 100);
    EXPECT_EQ(userInfo.nickname, "NewPlayer");
    EXPECT_EQ(userInfo.games, 0);
    EXPECT_EQ(userInfo.words, 0);
}

TEST_F(InfoServiceTest, GetUserInfoWithSingleGame)
{
    User user(50, "Player50");
    std::vector<Game> gameHistory{Game(999, 50, 42, 1)};

    EXPECT_CALL(*usersRepo, getUser(50)).WillOnce(Return(user));
    EXPECT_CALL(*gamesRepo, getGameHistoryByUserId(50)).WillOnce(Return(gameHistory));

    auto userInfo = service->getUserInfo(50);

    EXPECT_EQ(userInfo.id, 50);
    EXPECT_EQ(userInfo.games, 1);
    EXPECT_EQ(userInfo.words, 42);
}

TEST_F(InfoServiceTest, GetUserInfoMultipleUsersIndependent)
{
    User user1(1, "Alice");
    User user2(2, "Bob");

    std::vector<Game> history1{Game(1, 1, 20, 1), Game(2, 1, 30, 1)};

    std::vector<Game> history2{Game(3, 2, 15, 1)};

    EXPECT_CALL(*usersRepo, getUser(1)).WillOnce(Return(user1));
    EXPECT_CALL(*gamesRepo, getGameHistoryByUserId(1)).WillOnce(Return(history1));
    EXPECT_CALL(*usersRepo, getUser(2)).WillOnce(Return(user2));
    EXPECT_CALL(*gamesRepo, getGameHistoryByUserId(2)).WillOnce(Return(history2));

    auto info1 = service->getUserInfo(1);
    auto info2 = service->getUserInfo(2);

    EXPECT_EQ(info1.games, 2);
    EXPECT_EQ(info1.words, 50);
    EXPECT_EQ(info2.games, 1);
    EXPECT_EQ(info2.words, 15);
}

TEST_F(InfoServiceTest, GetGamesHistoryReturnsCorrectData)
{
    std::vector<Game> gameHistory{Game(1, 42, 10, 1), Game(2, 42, 15, 2), Game(3, 42, 5, 3)};

    EXPECT_CALL(*gamesRepo, getGameHistoryByUserId(42)).WillOnce(Return(gameHistory));

    auto result = service->getGamesHistory(42);

    EXPECT_EQ(result.size(), 3);
    EXPECT_EQ(result[0].gameId, 1);
    EXPECT_EQ(result[0].words, 10);
    EXPECT_EQ(result[0].place, 1);
    EXPECT_EQ(result[1].gameId, 2);
    EXPECT_EQ(result[1].words, 15);
    EXPECT_EQ(result[1].place, 2);
    EXPECT_EQ(result[2].gameId, 3);
    EXPECT_EQ(result[2].words, 5);
    EXPECT_EQ(result[2].place, 3);
}

TEST_F(InfoServiceTest, GetGamesHistoryEmptyWhenNoGames)
{
    std::vector<Game> emptyHistory;

    EXPECT_CALL(*gamesRepo, getGameHistoryByUserId(999)).WillOnce(Return(emptyHistory));

    auto result = service->getGamesHistory(999);

    EXPECT_EQ(result.size(), 0);
}

TEST_F(InfoServiceTest, GetGamesHistoryPreservesGameOrder)
{
    std::vector<Game> gameHistory{Game(100, 10, 50, 1), Game(101, 10, 25, 2), Game(102, 10, 75, 3)};

    EXPECT_CALL(*gamesRepo, getGameHistoryByUserId(10)).WillOnce(Return(gameHistory));

    auto result = service->getGamesHistory(10);

    EXPECT_EQ(result[0].gameId, 100);
    EXPECT_EQ(result[1].gameId, 101);
    EXPECT_EQ(result[2].gameId, 102);
}