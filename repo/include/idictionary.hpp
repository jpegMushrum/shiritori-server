#pragma once

#include <string>

#include "word.hpp"

class IDictionary
{
  public:
    virtual ~IDictionary() = default;
    virtual Word SearchWord(const std::string&) = 0;
};