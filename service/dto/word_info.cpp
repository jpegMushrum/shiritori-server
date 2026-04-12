#include "word_info.hpp"

WordInfo::WordInfo(std::string kanji, std::vector<std::string> readings,
                   std::set<std::string> partsOfSpeach, std::string meaning)
    : kanji(kanji), meaning(meaning),
      readings(std::vector<std::string>(readings.begin(), readings.end())),
      partsOfSpeach(std::set<std::string>(partsOfSpeach.begin(), partsOfSpeach.end()))
{
}