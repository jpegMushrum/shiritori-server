#pragma once

#include <set>
#include <string>
#include <vector>

class WordInfo
{
  public:
    WordInfo();
    WordInfo(std::string, std::vector<std::string>, std::set<std::string>, std::string);

    std::string kanji;
    std::vector<std::string> readings;
    std::set<std::string> partsOfSpeach;

    std::string meaning;
};