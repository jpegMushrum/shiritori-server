#pragma once

#include <chrono>
#include <map>
#include <mutex>
#include <random>
#include <shared_mutex>
#include <string>

using ull = unsigned long long;

struct SessionData
{
    ull userId;
    std::chrono::steady_clock::time_point createdAt;
};

class SessionManager
{
  public:
    static constexpr std::chrono::seconds SESSION_TTL{86400};

    SessionManager();

    std::string createSession(ull userId);

    ull getUserIdFromSession(const std::string& sessionId);

    bool sessionExists(const std::string& sessionId);

    void removeSession(const std::string& sessionId);

    void cleanupExpiredSessions();

  private:
    std::map<std::string, SessionData> sessions_;
    mutable std::shared_mutex mu_;
    std::mt19937 generator_;

    std::string generateSessionId();

    bool isSessionExpired(const SessionData& session) const;
};
