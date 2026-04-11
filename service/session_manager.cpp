#include "session_manager.hpp"
#include <chrono>

SessionManager::SessionManager()
    : generator_(std::chrono::system_clock::now().time_since_epoch().count())
{
}

std::string SessionManager::generateSessionId()
{
    std::uniform_int_distribution<ull> distribution(0, 15);
    std::string sessionId;
    for (int i = 0; i < 32; ++i)
    {
        ull digit = distribution(generator_);
        char hexChar = (digit < 10) ? ('0' + digit) : ('a' + (digit - 10));
        sessionId += hexChar;
    }
    return sessionId;
}

std::string SessionManager::createSession(ull userId)
{
    std::unique_lock<std::shared_mutex> lock(sessions_mutex_);

    std::string sessionId;
    do
    {
        sessionId = generateSessionId();
    } while (sessions_.find(sessionId) != sessions_.end());

    sessions_[sessionId] = userId;
    return sessionId;
}

ull SessionManager::getUserIdFromSession(const std::string& sessionId)
{
    std::shared_lock<std::shared_mutex> lock(sessions_mutex_);

    auto it = sessions_.find(sessionId);
    if (it != sessions_.end())
    {
        return it->second;
    }
    throw std::runtime_error("Session not found");
}

bool SessionManager::sessionExists(const std::string& sessionId)
{
    std::shared_lock<std::shared_mutex> lock(sessions_mutex_);
    return sessions_.find(sessionId) != sessions_.end();
}

void SessionManager::removeSession(const std::string& sessionId)
{
    std::unique_lock<std::shared_mutex> lock(sessions_mutex_);
    sessions_.erase(sessionId);
}
