#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <boost/asio.hpp>
#include <chrono>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "game_fabric.hpp"
#include "games_controller.hpp"
#include "games_repo.hpp"
#include "igames_controller.hpp"
#include "iinfo_controller.hpp"
#include "info_controller.hpp"
#include "info_service.hpp"
#include "jisho.hpp"
#include "server.hpp"
#include "task_queue.hpp"
#include "users_repo.hpp"

using ull = unsigned long long;
using tcp = boost::asio::ip::tcp;

class TestClient
{
  public:
    TestClient(const std::string& host, unsigned short port)
        : host_(host), port_(port), socket_(io_)
    {
    }

    ~TestClient()
    {
        disconnect();
    }

    bool connect()
    {
        try
        {
            tcp::resolver resolver(io_);
            auto results = resolver.resolve(host_, std::to_string(port_));

            boost::asio::connect(socket_, results);

            // Set socket to non-blocking mode
            socket_.non_blocking(true);

            return true;
        }
        catch (const std::exception& e)
        {
            std::cerr << "Connection failed: " << e.what() << std::endl;
            return false;
        }
    }

    bool disconnect()
    {
        try
        {
            if (socket_.is_open())
            {
                socket_.close();
            }
            return true;
        }
        catch (const std::exception& e)
        {
            std::cerr << "Disconnect failed: " << e.what() << std::endl;
            return false;
        }
    }

    bool sendRequest(const std::string& request)
    {
        try
        {
            std::string msg = request + "\n";
            boost::asio::write(socket_, boost::asio::buffer(msg));
            return true;
        }
        catch (const std::exception& e)
        {
            std::cerr << "Send failed: " << e.what() << std::endl;
            return false;
        }
    }

    std::string receiveResponse(int maxRetries = 20, int delayMs = 50)
    {
        std::string response;
        try
        {
            boost::asio::streambuf buffer;
            int retries = 0;

            while (retries < maxRetries)
            {
                try
                {
                    size_t bytesRead =
                        boost::asio::read(socket_, buffer, boost::asio::transfer_at_least(1));

                    if (bytesRead > 0)
                    {
                        std::istream is(&buffer);
                        std::getline(is, response);
                        return response;
                    }
                }
                catch (boost::system::system_error& e)
                {
                    if (e.code() != boost::asio::error::would_block)
                    {
                        throw e;
                    }
                    // Timeout, retry
                }

                retries++;
                std::this_thread::sleep_for(std::chrono::milliseconds(delayMs));
            }
        }
        catch (const std::exception& e)
        {
            std::cerr << "Receive failed: " << e.what() << std::endl;
        }

        return "";
    }

  private:
    std::string host_;
    unsigned short port_;
    boost::asio::io_context io_;
    tcp::socket socket_;
};

class FullServerTest : public ::testing::Test
{
  protected:
    static constexpr unsigned short TEST_PORT = 9999;
    static const std::string TEST_HOST;

    void SetUp() override
    {
        std::string dbPath = std::string(std::tmpnam(nullptr)) + ".db";

        auto jisho = std::make_shared<JishoDict>();
        usersRepo_ = std::make_shared<UsersRepo>(dbPath);
        gamesRepo_ = std::make_shared<GamesRepo>(dbPath);

        auto infoService = std::make_unique<InfoService>(usersRepo_, gamesRepo_);
        auto gameFabric = std::make_unique<GameFabric>(jisho, gamesRepo_);

        taskQueue_ = std::make_shared<TaskQueue>(std::thread::hardware_concurrency());
        infoCtr_ = std::make_shared<InfoController>(taskQueue_, std::move(infoService));
        gamesCtr_ = std::make_shared<GamesController>(taskQueue_, std::move(gameFabric));

        serverThread_ = std::thread([this]() { runServer(); });

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    void TearDown() override
    {
        stopServer();

        if (serverThread_.joinable())
        {
            serverThread_.join();
        }
    }

    void runServer()
    {
        try
        {
            Server server(io_, TEST_PORT, infoCtr_, gamesCtr_);
            server.startAccepting();
        }
        catch (const std::exception& e)
        {
            std::cerr << "Server error: " << e.what() << std::endl;
        }
    }

    void stopServer()
    {
        io_.stop();
    }

    bool createAndConnectClient(TestClient& client)
    {
        if (!client.connect())
        {
            return false;
        }
        clients_.push_back(&client);
        return true;
    }

    std::string sendAndReceive(TestClient& client, const std::string& request)
    {
        if (!client.sendRequest(request))
        {
            return "";
        }
        return client.receiveResponse();
    }

    bool responseContains(const std::string& response, const std::string& substring)
    {
        return response.find(substring) != std::string::npos;
    }

    bool responseDoesNotContain(const std::string& response, const std::string& substring)
    {
        return response.find(substring) == std::string::npos;
    }

    bool isErrorResponse(const std::string& response)
    {
        return response.find("Error") != std::string::npos;
    }

    boost::asio::io_context io_;
    std::thread serverThread_;

    std::shared_ptr<IUsersRepo> usersRepo_;
    std::shared_ptr<IGamesRepo> gamesRepo_;
    std::shared_ptr<IInfoController> infoCtr_;
    std::shared_ptr<IGamesController> gamesCtr_;
    std::shared_ptr<ITaskQueue> taskQueue_;

    std::vector<TestClient*> clients_;
};

const std::string FullServerTest::TEST_HOST = "127.0.0.1";

TEST_F(FullServerTest, ClientCanConnectToServer)
{
    TestClient client(TEST_HOST, TEST_PORT);
    EXPECT_TRUE(client.connect());
    client.disconnect();
}

TEST_F(FullServerTest, CanAddUser)
{
    TestClient client(TEST_HOST, TEST_PORT);
    bool connected = createAndConnectClient(client);
    ASSERT_TRUE(connected);

    std::string response = sendAndReceive(client, "login TestUser");

    EXPECT_FALSE(isErrorResponse(response));

    client.disconnect();
}

TEST_F(FullServerTest, CanGetUserInfo)
{
    TestClient client(TEST_HOST, TEST_PORT);
    bool connected = createAndConnectClient(client);
    ASSERT_TRUE(connected);

    std::string loginResponse = sendAndReceive(client, "login Alice");
    bool gotError = isErrorResponse(loginResponse);
    EXPECT_FALSE(gotError);
    std::string sessionId = loginResponse;

    std::string getResponse = sendAndReceive(client, "getUserInfo " + sessionId);
    gotError = isErrorResponse(getResponse);

    EXPECT_FALSE(gotError);

    EXPECT_FALSE(gotError);
    EXPECT_TRUE(responseContains(getResponse, "Alice"));

    client.disconnect();
}

TEST_F(FullServerTest, ErrorWhenGettingNonexistentUser)
{
    TestClient client(TEST_HOST, TEST_PORT);
    ASSERT_TRUE(createAndConnectClient(client));

    std::string response = sendAndReceive(client, "getUserInfo invalidSessionId123");
    EXPECT_TRUE(isErrorResponse(response));

    client.disconnect();
}

TEST_F(FullServerTest, MultipleClientsCanConnect)
{
    TestClient client1(TEST_HOST, TEST_PORT);
    TestClient client2(TEST_HOST, TEST_PORT);
    TestClient client3(TEST_HOST, TEST_PORT);

    ASSERT_TRUE(createAndConnectClient(client1));
    ASSERT_TRUE(createAndConnectClient(client2));
    ASSERT_TRUE(createAndConnectClient(client3));

    std::string response1 = sendAndReceive(client1, "login User1");
    std::string response2 = sendAndReceive(client2, "login User2");
    std::string response3 = sendAndReceive(client3, "login User3");

    EXPECT_FALSE(isErrorResponse(response1));
    EXPECT_FALSE(isErrorResponse(response2));
    EXPECT_FALSE(isErrorResponse(response3));

    client1.disconnect();
    client2.disconnect();
    client3.disconnect();
}

TEST_F(FullServerTest, InvalidCommandReturnsError)
{
    TestClient client(TEST_HOST, TEST_PORT);
    ASSERT_TRUE(createAndConnectClient(client));

    std::string response = sendAndReceive(client, "invalidCommand arg1 arg2");

    EXPECT_TRUE(isErrorResponse(response));

    client.disconnect();
}

TEST_F(FullServerTest, ResponseFormatIsCorrect)
{
    TestClient client(TEST_HOST, TEST_PORT);
    ASSERT_TRUE(createAndConnectClient(client));

    std::string response = sendAndReceive(client, "login TestUser");

    EXPECT_FALSE(response.empty());

    client.disconnect();
}

TEST_F(FullServerTest, SequentialRequestsFromSingleClient)
{
    TestClient client(TEST_HOST, TEST_PORT);
    ASSERT_TRUE(createAndConnectClient(client));

    std::string response1 = sendAndReceive(client, "login Player1");
    EXPECT_FALSE(isErrorResponse(response1));
    std::string session1 = response1;

    std::string response2 = sendAndReceive(client, "login Player2");
    EXPECT_FALSE(isErrorResponse(response2));
    std::string session2 = response2;

    std::string response3 = sendAndReceive(client, "getUserInfo " + session1);
    std::cout << "d|" << response3 << '\n';
    EXPECT_FALSE(isErrorResponse(response3));

    client.disconnect();
}

TEST_F(FullServerTest, AddUserWithEmptyName)
{
    TestClient client(TEST_HOST, TEST_PORT);
    ASSERT_TRUE(createAndConnectClient(client));

    std::string response = sendAndReceive(client, "login ");

    EXPECT_TRUE(isErrorResponse(response));

    client.disconnect();
}

TEST_F(FullServerTest, AddUserWithVeryLongName)
{
    TestClient client(TEST_HOST, TEST_PORT);
    ASSERT_TRUE(createAndConnectClient(client));

    std::string longName(1000, 'A');
    std::string response = sendAndReceive(client, "login " + longName);

    EXPECT_FALSE(isErrorResponse(response));

    client.disconnect();
}

TEST_F(FullServerTest, AddUserWithSpecialCharacters)
{
    TestClient client(TEST_HOST, TEST_PORT);
    ASSERT_TRUE(createAndConnectClient(client));

    std::string response = sendAndReceive(client, "login User@#$%^&*(");

    EXPECT_FALSE(isErrorResponse(response));

    client.disconnect();
}

TEST_F(FullServerTest, CommandWithoutArguments)
{
    TestClient client(TEST_HOST, TEST_PORT);
    ASSERT_TRUE(createAndConnectClient(client));

    std::string response = sendAndReceive(client, "login");

    EXPECT_TRUE(isErrorResponse(response));

    client.disconnect();
}

TEST_F(FullServerTest, GetUserWithZeroId)
{
    TestClient client(TEST_HOST, TEST_PORT);
    ASSERT_TRUE(createAndConnectClient(client));

    std::string response = sendAndReceive(client, "getUserInfo 0");

    EXPECT_TRUE(isErrorResponse(response));

    client.disconnect();
}

TEST_F(FullServerTest, GetUserWithNegativeId)
{
    TestClient client(TEST_HOST, TEST_PORT);
    ASSERT_TRUE(createAndConnectClient(client));

    std::string response = sendAndReceive(client, "getUserInfo -1");

    EXPECT_TRUE(isErrorResponse(response));

    client.disconnect();
}

TEST_F(FullServerTest, RecoverFromErrorAndContinue)
{
    TestClient client(TEST_HOST, TEST_PORT);
    ASSERT_TRUE(createAndConnectClient(client));

    std::string errorResponse = sendAndReceive(client, "invalidCommand");
    EXPECT_TRUE(isErrorResponse(errorResponse));

    std::string validResponse = sendAndReceive(client, "login RecoveryUser");
    EXPECT_FALSE(isErrorResponse(validResponse));

    client.disconnect();
}

TEST_F(FullServerTest, MultipleUsersWithSameName)
{
    TestClient client(TEST_HOST, TEST_PORT);
    ASSERT_TRUE(createAndConnectClient(client));

    std::string response1 = sendAndReceive(client, "login Duplicate");
    EXPECT_FALSE(isErrorResponse(response1));
    std::string session1 = response1;

    std::string response2 = sendAndReceive(client, "login Duplicate");
    EXPECT_FALSE(isErrorResponse(response2));
    std::string session2 = response2;

    std::string getResponse1 = sendAndReceive(client, "getUserInfo " + session1);
    std::string getResponse2 = sendAndReceive(client, "getUserInfo " + session2);

    EXPECT_TRUE(responseContains(getResponse1, "Duplicate"));
    EXPECT_TRUE(responseContains(getResponse2, "Duplicate"));

    client.disconnect();
}

TEST_F(FullServerTest, CommandWithExtraWhitespace)
{
    TestClient client(TEST_HOST, TEST_PORT);
    ASSERT_TRUE(createAndConnectClient(client));

    std::string response = sendAndReceive(client, "login    PaddedUser   ");
    EXPECT_FALSE(isErrorResponse(response));

    client.disconnect();
}

TEST_F(FullServerTest, CreateGameSuccessfully)
{
    TestClient client(TEST_HOST, TEST_PORT);
    ASSERT_TRUE(createAndConnectClient(client));

    std::string loginResponse = sendAndReceive(client, "login GameCreator");
    EXPECT_FALSE(isErrorResponse(loginResponse));
    std::string sessionId = loginResponse;

    std::string gameResponse = sendAndReceive(client, "startNewGame " + sessionId);
    EXPECT_FALSE(isErrorResponse(gameResponse));
    EXPECT_FALSE(gameResponse.empty());

    client.disconnect();
}

TEST_F(FullServerTest, GetActiveGames)
{
    TestClient client(TEST_HOST, TEST_PORT);
    ASSERT_TRUE(createAndConnectClient(client));

    std::string loginResponse = sendAndReceive(client, "login Player1");
    EXPECT_FALSE(isErrorResponse(loginResponse));
    std::string sessionId = loginResponse;

    sendAndReceive(client, "startNewGame " + sessionId);

    std::string activeGamesResponse = sendAndReceive(client, "getActiveGames");
    EXPECT_FALSE(isErrorResponse(activeGamesResponse));

    client.disconnect();
}

TEST_F(FullServerTest, AddPlayerToGame)
{
    TestClient client(TEST_HOST, TEST_PORT);
    ASSERT_TRUE(createAndConnectClient(client));

    std::string creatorLogin = sendAndReceive(client, "login Creator");
    std::string playerLogin = sendAndReceive(client, "login Player");
    EXPECT_FALSE(isErrorResponse(creatorLogin));
    EXPECT_FALSE(isErrorResponse(playerLogin));

    std::string creatorSession = creatorLogin;
    std::string playerSession = playerLogin;

    std::string gameResponse = sendAndReceive(client, "startNewGame " + creatorSession);
    EXPECT_FALSE(isErrorResponse(gameResponse));

    std::string addPlayerResponse =
        sendAndReceive(client, "addPlayerToGame " + playerSession + " 0");
    EXPECT_FALSE(isErrorResponse(addPlayerResponse));

    std::string getGameInfoResponse = sendAndReceive(client, "getGameInfo 0");
    EXPECT_TRUE(responseContains(getGameInfoResponse, "2") ||
                responseContains(getGameInfoResponse, "1"));

    client.disconnect();
}

TEST_F(FullServerTest, GetGamesHistory)
{
    TestClient client(TEST_HOST, TEST_PORT);
    ASSERT_TRUE(createAndConnectClient(client));

    std::string loginResponse = sendAndReceive(client, "login HistoryPlayer");
    EXPECT_FALSE(isErrorResponse(loginResponse));
    std::string sessionId = loginResponse;

    std::string historyResponse = sendAndReceive(client, "getGamesHistory " + sessionId);
    EXPECT_FALSE(isErrorResponse(historyResponse));

    client.disconnect();
}

TEST_F(FullServerTest, StopGameSuccessfully)
{
    TestClient client(TEST_HOST, TEST_PORT);
    ASSERT_TRUE(createAndConnectClient(client));

    std::string loginResponse = sendAndReceive(client, "login GameAdmin");
    EXPECT_FALSE(isErrorResponse(loginResponse));
    std::string sessionId = loginResponse;

    std::string gameResponse = sendAndReceive(client, "startNewGame " + sessionId);
    EXPECT_FALSE(isErrorResponse(gameResponse));

    std::string stopResponse = sendAndReceive(client, "stopGame " + sessionId + " 0");
    EXPECT_FALSE(isErrorResponse(stopResponse));

    client.disconnect();
}

TEST_F(FullServerTest, GameStatisticsAfterStop)
{
    TestClient client(TEST_HOST, TEST_PORT);
    ASSERT_TRUE(createAndConnectClient(client));

    std::string user1Login = sendAndReceive(client, "login StatPlayer1");
    std::string user2Login = sendAndReceive(client, "login StatPlayer2");
    EXPECT_FALSE(isErrorResponse(user1Login));
    EXPECT_FALSE(isErrorResponse(user2Login));

    std::string session1 = user1Login;
    std::string session2 = user2Login;

    std::string gameResponse = sendAndReceive(client, "startNewGame " + session1);
    EXPECT_FALSE(isErrorResponse(gameResponse));

    sendAndReceive(client, "addPlayerToGame " + session2 + " 0");
    sendAndReceive(client, "stopGame " + session1 + " 0");

    std::string history1 = sendAndReceive(client, "getGamesHistory " + session1);
    std::string history2 = sendAndReceive(client, "getGamesHistory " + session2);

    EXPECT_FALSE(isErrorResponse(history1));
    EXPECT_FALSE(isErrorResponse(history2));

    client.disconnect();
}

TEST_F(FullServerTest, SimultaneousRequestsFromMultipleClients)
{
    TestClient client1(TEST_HOST, TEST_PORT);
    TestClient client2(TEST_HOST, TEST_PORT);

    ASSERT_TRUE(createAndConnectClient(client1));
    ASSERT_TRUE(createAndConnectClient(client2));

    std::string r1 = sendAndReceive(client1, "login SyncUser1");
    std::string r2 = sendAndReceive(client2, "login SyncUser2");

    EXPECT_FALSE(isErrorResponse(r1));
    EXPECT_FALSE(isErrorResponse(r2));

    std::string session1 = r1;
    std::string session2 = r2;

    std::string info1 = sendAndReceive(client1, "getUserInfo " + session1);
    std::string info2 = sendAndReceive(client2, "getUserInfo " + session2);

    EXPECT_TRUE(responseContains(info1, "SyncUser1"));
    EXPECT_TRUE(responseContains(info2, "SyncUser2"));

    client1.disconnect();
    client2.disconnect();
}
