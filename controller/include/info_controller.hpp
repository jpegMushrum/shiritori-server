#pragma once

#include <functional>
#include <vector>

#include "game_info.hpp"
#include "iinfo_controller.hpp"
#include "iinfo_service.hpp"
#include "task_queue.hpp"
#include "user_info.hpp"

using ull = unsigned long long;

class InfoController : public IInfoController
{
  public:
    InfoController(std::shared_ptr<ITaskQueue>, std::unique_ptr<IInfoService>);

    void getUserInfo(ull, std::function<void(UserInfo)>) override;
    void addUser(const std::string&, std::function<void(ull)>) override;

    void getGamesHistory(ull, std::function<void(std::vector<GameInfo>)>) override;

  private:
    std::shared_ptr<ITaskQueue> taskQueue_;
    std::unique_ptr<IInfoService> infoService_;
};