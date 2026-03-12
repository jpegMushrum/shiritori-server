#pragma once

#include <string>
#include <vector>

#include "word.hpp"

class IDictionary
{
  public:
    virtual ~IDictionary() = default;
    virtual std::vector<Word> searchWord(const std::string&) = 0;
};