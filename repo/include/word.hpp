#pragma once

#include <set>
#include <string>
#include <vector>

class Word
{
  public:
    Word();

    std::string kanji;
    std::vector<std::string> readings;
    std::set<std::string> partsOfSpeach;

    std::string meaning;
};