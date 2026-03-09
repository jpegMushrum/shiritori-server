#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <unordered_set>
#include <vector>

#include "game_info.hpp"
#include "handle_word_status.hpp"
#include "jisho.hpp"
#include "user.hpp"
#include "word.hpp"

using ull = unsigned long long;

class GameSession
{
  public:
    void addUser(ull);
    HandleWordStatus handleWord(ull, const std::string&);

    GameInfo GetInfo();
    GameSession(ull, std::shared_ptr<IDictionary>);

  private:
    std::unordered_set<ull> players_;
    std::vector<Word> words_;

    std::shared_ptr<IDictionary> dict_;

    std::mutex mu_;
    GameInfo info_;
};