#pragma once

#include <algorithm>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_set>
#include <vector>

#include "game.hpp"
#include "game_context.hpp"
#include "games_repo.hpp"
#include "handle_word_status.hpp"
#include "jisho.hpp"
#include "player_score.hpp"
#include "user.hpp"
#include "word.hpp"

using ull = unsigned long long;

class GameSession
{
  public:
    GameSession(ull, ull, std::shared_ptr<IDictionary>, std::shared_ptr<GamesRepo>);
    ~GameSession();

    void addUser(ull);
    HandleWordStatus handleWord(ull, const std::string&);
    void stopGame();

    GameContext GetInfo();

  private:
    void saveStats();

    std::unordered_set<PlayerScore> players_;
    std::vector<Word> words_;

    std::shared_ptr<IDictionary> dict_;
    std::shared_ptr<GamesRepo> repo_;

    bool stop_;
    std::mutex mu_;
    GameContext ctx_;
};