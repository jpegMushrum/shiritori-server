#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <future>

#include "iinfo_service.hpp"
#include "info_controller.hpp"
#include "task_queue.hpp"
#include "user_info.hpp"

using ::testing::_;
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

        mockService = new MockInfoService();

        controller =
            std::make_unique<InfoController>(taskQueue, std::unique_ptr<IInfoService>(mockService));
    }

    std::shared_ptr<TaskQueue> taskQueue;
    MockInfoService* mockService;
    std::unique_ptr<InfoController> controller;
};

TEST_F(InfoControllerTest, GetUserInfoCallsServiceAndReturnsResult)
{
    UserInfo expected(42, "a", 42, 42);

    EXPECT_CALL(*mockService, getUserInfo(42)).Times(1).WillOnce(Return(expected));

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
    EXPECT_CALL(*mockService, addUser("Alice")).Times(1).WillOnce(Return(100));

    std::promise<ull> promise;
    auto future = promise.get_future();

    controller->addUser("Alice", [&](ull id) { promise.set_value(id); });

    ull result = future.get();

    EXPECT_EQ(result, 100);
}