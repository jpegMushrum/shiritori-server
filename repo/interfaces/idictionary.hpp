#pragma once

#include <string>
#include <vector>

#include "word.hpp"

class IDictionary
{
  public:
    virtual ~IDictionary() = default;
    virtual std::vector<Word> SearchWord(const std::string&) = 0;
};