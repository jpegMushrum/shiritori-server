#pragma once

#include "dictionary.hpp"
#include "word.hpp"

class JishoDict : IDictionary
{
  public:
    Word SearchWord(const std::string&);

  private:
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);
};