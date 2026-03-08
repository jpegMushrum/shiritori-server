#pragma once

#include <string>

#include "word.hpp"

class IDictionary
{
  public:
    virtual Word SearchWord(const std::string&);

  private:
};