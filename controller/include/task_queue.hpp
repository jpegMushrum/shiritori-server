#pragma once

#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>

#include "itask_queue.hpp"

class TaskQueue : public ITaskQueue
{
  public:
    TaskQueue(unsigned int);
    ~TaskQueue();
    void addTask(std::function<void()>) override;

  private:
    void startWorkerLoop();

    std::condition_variable cv_;
    std::mutex mu_;
    bool stop_;

    std::vector<std::unique_ptr<std::thread>> workers_;
    std::queue<std::function<void()>> tasks_;
};