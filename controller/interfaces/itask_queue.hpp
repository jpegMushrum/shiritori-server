#pragma once

#include <functional>

class ITaskQueue
{
  public:
    virtual ~ITaskQueue() = default;

    virtual void addTask(std::function<void()>) = 0;
};