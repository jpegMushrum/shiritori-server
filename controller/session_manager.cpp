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

bool SessionManager::isSessionExpired(const SessionData& session) const
{
    auto now = std::chrono::steady_clock::now();
    auto age = std::chrono::duration_cast<std::chrono::seconds>(now - session.createdAt);
    return age > SESSION_TTL;
}

std::string SessionManager::createSession(ull userId)
{
    std::unique_lock lock(mu_);

    cleanupExpiredSessions();

    std::string sessionId;
    do
    {
        sessionId = generateSessionId();
    } while (sessions_.find(sessionId) != sessions_.end());

    sessions_[sessionId] = {userId, std::chrono::steady_clock::now()};
    return sessionId;
}

ull SessionManager::getUserIdFromSession(const std::string& sessionId)
{
    std::shared_lock lock(mu_);

    auto it = sessions_.find(sessionId);
    if (it != sessions_.end())
    {
        if (!isSessionExpired(it->second))
        {
            return it->second.userId;
        }
    }
    throw std::runtime_error("Session not found");
}

bool SessionManager::sessionExists(const std::string& sessionId)
{
    std::shared_lock lock(mu_);

    auto it = sessions_.find(sessionId);
    if (it != sessions_.end() && !isSessionExpired(it->second))
    {
        return true;
    }
    return false;
}

void SessionManager::removeSession(const std::string& sessionId)
{
    std::unique_lock lock(mu_);
    sessions_.erase(sessionId);
}

void SessionManager::cleanupExpiredSessions()
{
    // Note: This method assumes the caller already holds the lock
    auto now = std::chrono::steady_clock::now();
    auto it = sessions_.begin();
    while (it != sessions_.end())
    {
        if (isSessionExpired(it->second))
        {
            it = sessions_.erase(it);
        }
        else
        {
            ++it;
        }
    }
}
