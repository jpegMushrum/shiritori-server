#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include "user.hpp"
#include "word.hpp"

using ull = unsigned long long;

class GameSession {
public:

void addUser(ull id);
void handleWord(ull id, std::string word);

private:
    std::unordered_map<ull, User> players_;
    std::vector<Word> words_;
};