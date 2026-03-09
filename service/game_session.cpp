#include <future>

#include "game_session.hpp"

using ull = unsigned long long;

GameSession::GameSession(ull id, std::shared_ptr<IDictionary> dict)
    : info_(id, 0, 0, 16), dict_(dict)
{
}

void GameSession::addUser(ull id)
{
    std::lock_guard lock(mu_);

    players_.emplace(id);
    info_.playersCount++;
}

HandleWordStatus GameSession::handleWord(ull id, const std::string& word)
{
    std::promise<Word> promise;
    auto future =
        std::async(std::launch::async, [this, word]() { return dict_->SearchWord(word); });

    // тут будут проверки

    auto response = future.get();
    return HandleWordStatus::OK;
}

GameInfo GameSession::GetInfo()
{
    std::lock_guard lock(mu_);
    return info_;
}