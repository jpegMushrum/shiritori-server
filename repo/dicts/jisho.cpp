
#include <iostream>

#include "jisho.hpp"

using json = nlohmann::json;

size_t JishoDict::WriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
    size_t totalSize = size * nmemb;
    std::string* str = static_cast<std::string*>(userp);
    str->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}

Word JishoDict::SearchWord(const std::string& word)
{
    CURL* curl = curl_easy_init();
    if (!curl)
    {
        throw std::runtime_error("Failed to initialize cURL");
    }

    char* escaped = curl_easy_escape(curl, word.c_str(), word.size());
    if (!escaped)
    {
        curl_easy_cleanup(curl);
        throw std::runtime_error("Failed to escape URL");
    }

    std::string url = "https://jisho.org/api/v1/search/words?keyword=" + std::string(escaped);
    std::cout << url << "\n";
    curl_free(escaped);

    std::string response;

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, JishoDict::WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0");

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK)
    {
        throw std::runtime_error(std::string("cURL error: ") + curl_easy_strerror(res));
    }

    std::cout << "d| " << response << '\n';
    auto jsonResponse = json::parse(response);
    return Word();
}