#pragma once

#include <functional>
#include <vector>

#include "iinfo_service.hpp"
#include "task_queue.hpp"
#include "user_info.hpp"

using ull = unsigned long long;

class InfoController
{
  public:
    InfoController(std::shared_ptr<TaskQueue>, std::unique_ptr<IInfoService>);

    void getUserInfo(ull, std::function<void(UserInfo)>);
    void addUser(const std::string&, std::function<void(ull)>);

  private:
    std::shared_ptr<TaskQueue> taskQueue_;
    std::unique_ptr<IInfoService> infoService_;
};