#include "task_queue.hpp"

void TaskQueue::startWorkerLoop()
{
    while (true)
    {
        std::function<void()> task;
        {
            {
                std::unique_lock<std::mutex> lock(mu_);
                cv_.wait(lock, [this]() { return !tasks_.empty() || stop_; });

                if (tasks_.empty())
                {
                    if (stop_)
                    {
                        break;
                    }
                    continue;
                }

                task = std::move(tasks_.front());
                tasks_.pop();
            }

            try
            {
                task();
            }
            catch (...)
            {
            }
        }
    }
}

TaskQueue::TaskQueue(unsigned int workersCount) : stop_(false)
{
    for (int i = 0; i < workersCount; i++)
    {
        workers_.push_back(std::make_unique<std::thread>([this]() { startWorkerLoop(); }));
    }
}

TaskQueue::~TaskQueue()
{
    {
        std::lock_guard lock(mu_);
        stop_ = true;
    }

    cv_.notify_all();

    for (auto& t : workers_)
    {
        if (t->joinable())
        {
            t->join();
        }
    }
}

void TaskQueue::addTask(std::function<void()> cb)
{
    {
        std::lock_guard<std::mutex> lock(mu_);
        tasks_.push(cb);
    }

    cv_.notify_one();
}