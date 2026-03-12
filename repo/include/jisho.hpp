#pragma once

#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <vector>

#include "idictionary.hpp"
#include "word.hpp"

using json = nlohmann::json;

class JishoDict : public IDictionary
{
  public:
    std::vector<Word> searchWord(const std::string&) override;

  private:
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);
    static inline std::vector<Word> jsonToWords(json&);
};