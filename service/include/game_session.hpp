#pragma once

#include <algorithm>
#include <boost/locale.hpp>
#include <functional>
#include <memory>
#include <mutex>
#include <random>
#include <string>
#include <unordered_set>
#include <vector>

#include "game.hpp"
#include "game_context.hpp"
#include "games_repo.hpp"
#include "handle_word_status.hpp"
#include "igame_session.hpp"
#include "jisho.hpp"
#include "mapper.hpp"
#include "player_score.hpp"
#include "user.hpp"
#include "word.hpp"
#include "word_info.hpp"

using ull = unsigned long long;

class GameSession : public IGameSession
{
  public:
    GameSession(ull, ull, std::shared_ptr<IDictionary>, std::shared_ptr<IGamesRepo>, char32_t);
    ~GameSession() override;

    void addUser(ull) override;
    HandleWordStatus handleWord(ull, const std::string&) override;
    void stopGame() override;

    GameContext getInfo() override;
    void subscribe(std::function<void(WordInfo)>) override;

  private:
    void saveStats();

    std::unordered_set<PlayerScore> players_;
    std::vector<Word> words_;

    std::shared_ptr<IDictionary> dict_;
    std::shared_ptr<IGamesRepo> repo_;

    bool stop_;
    std::mutex mu_;
    GameContext ctx_;

    std::vector<std::function<void(WordInfo)>> subscriptions_;
    void throwUpdate(WordInfo);
};