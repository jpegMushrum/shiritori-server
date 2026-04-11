#pragma once

#include <map>
#include <mutex>
#include <random>
#include <shared_mutex>
#include <string>

using ull = unsigned long long;

class SessionManager
{
  public:
    SessionManager();

    std::string createSession(ull userId);

    ull getUserIdFromSession(const std::string& sessionId);

    bool sessionExists(const std::string& sessionId);

    void removeSession(const std::string& sessionId);

  private:
    std::map<std::string, ull> sessions_;
    mutable std::shared_mutex sessions_mutex_;
    std::mt19937 generator_;

    std::string generateSessionId();
};
