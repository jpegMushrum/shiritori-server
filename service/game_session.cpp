#include "game_session.hpp"

using ull = unsigned long long;

GameSession::GameSession(ull id, ull adminId, std::shared_ptr<IDictionary> dict,
                         std::shared_ptr<IGamesRepo> repo, char32_t startKana)
    : dict_(dict), repo_(repo), stop_(false)
{
    ctx_ = GameContext(id, 0, 0, adminId, startKana);
}

void GameSession::stopGame()
{
    std::lock_guard lock(mu_);
    stop_ = true;
}

GameSession::~GameSession()
{
    saveStats();
}

void GameSession::saveStats()
{
    std::lock_guard lock(mu_);

    std::vector<PlayerScore> players(players_.begin(), players_.end());
    std::vector<Game> gameParts;

    std::sort(players.begin(), players.end(), PlayerScore::psCmp);

    int place = 1;
    for (auto& usr : players)
    {
        gameParts.emplace_back(Game(ctx_.id, usr.userId, usr.score, place));
        ++place;
    }

    repo_->putGame(gameParts);
}

void GameSession::addUser(ull id)
{
    std::lock_guard lock(mu_);

    PlayerScore player;
    player.userId = id;

    players_.insert(player);
    ctx_.playersCount++;
}

GameContext GameSession::getInfo()
{
    std::lock_guard lock(mu_);
    return ctx_;
}

char32_t toHiragana(char32_t c)
{
    if (c >= 0x30A1 && c <= 0x30F6)
        return c - 0x60;

    return c;
}

std::optional<Word> findRelevantWord(const std::string& word, const std::vector<Word>& wordsList)
{
    for (auto& w : wordsList)
    {
        if ((w.kanji == word || w.readings.contains(word)) &&
            w.readings.begin() != w.readings.end())
        {
            return w;
        }
    }

    return std::nullopt;
}

bool isWordDoubled(const std::string& word, const std::vector<Word>& usedWords)
{
    for (auto& usedWord : usedWords)
    {
        if (usedWord.kanji == word || usedWord.readings.contains(word))
        {
            return true;
        }
    }

    return false;
}

bool isJapaneseChar(char32_t c)
{
    return (c >= 0x3040 && c <= 0x309F) || // Hiragana
           (c >= 0x30A0 && c <= 0x30FF) || // Katakana
           (c >= 0x31F0 && c <= 0x31FF) || // Katakana ext
           (c >= 0x3400 && c <= 0x4DBF) || // Kanji ext A
           (c >= 0x4E00 && c <= 0x9FFF);   // Kanji
}

bool isJapaneseWord(const std::u32string& u32)
{
    for (char32_t c : u32)
        if (!isJapaneseChar(c))
            return false;

    return true;
}

bool isSmallKana(char32_t c)
{
    switch (c)
    {
    case U'ゃ':
    case U'ゅ':
    case U'ょ':
    case U'ぁ':
    case U'ぃ':
    case U'ぅ':
    case U'ぇ':
    case U'ぉ':
    case U'ャ':
    case U'ュ':
    case U'ョ':
    case U'ァ':
    case U'ィ':
    case U'ゥ':
    case U'ェ':
    case U'ォ':
        return true;
    }
    return false;
}

bool isLongVowel(char32_t c)
{
    return c == U'ー';
}

std::u32string conv_utf8_to_utf32(const std::string& s)
{
    return boost::locale::conv::utf_to_utf<char32_t>(s);
}

std::optional<char32_t> getLastKana(const std::u32string& u32)
{
    if (u32.empty())
        return std::nullopt;

    for (int i = u32.size() - 1; i >= 0; --i)
    {
        char32_t c = u32[i];

        if (isLongVowel(c))
            continue;

        if (isSmallKana(c))
        {
            if (i > 0)
                return u32[i - 1];
        }

        return c;
    }

    return std::nullopt;
}

std::optional<char32_t> getFirstKana(const std::u32string& u32)
{
    if (u32.empty())
        return std::nullopt;

    for (int i = 0; i < u32.size(); ++i)
    {
        char32_t c = u32[i];

        if (isLongVowel(c) || isSmallKana(c))
        {
            continue;
        }

        return c;
    }

    return std::nullopt;
}

HandleWordStatus GameSession::handleWord(ull id, const std::string& word)
{
    std::u32string u32word = conv_utf8_to_utf32(word);

    {
        std::lock_guard lock(mu_);
        if (stop_)
        {
            return HandleWordStatus::GAME_STOPPED;
        }

        auto it = players_.find({id});
        if (it == players_.end())
        {
            return HandleWordStatus::NO_FOUND_PLAYER;
        }
        PlayerScore ps = *it;

        if (ctx_.lastPlayerId == id)
        {
            return HandleWordStatus::WRONG_ORDER;
        }

        if (!isJapaneseWord(u32word))
        {
            return HandleWordStatus::NOT_JAPANESE_WORD;
        }

        if (isWordDoubled(word, words_))
        {
            return HandleWordStatus::GOT_DOUBLED_WORD;
        }
    }

    std::vector<Word> response;
    try
    {
        response = dict_->searchWord(word);
    }
    catch (const std::exception& e)
    {
        return HandleWordStatus::NO_FOUND_WORD;
    }

    {
        std::lock_guard lock(mu_);

        auto it = players_.find({id});
        if (it == players_.end())
        {
            return HandleWordStatus::NO_FOUND_PLAYER;
        }
        PlayerScore ps = *it;

        if (ctx_.lastPlayerId == id)
        {
            return HandleWordStatus::WRONG_ORDER;
        }

        if (!isJapaneseWord(u32word))
        {
            return HandleWordStatus::NOT_JAPANESE_WORD;
        }

        if (isWordDoubled(word, words_))
        {
            return HandleWordStatus::GOT_DOUBLED_WORD;
        }

        std::optional<Word> relevantWord = findRelevantWord(word, response);
        if (!relevantWord)
        {
            return HandleWordStatus::NO_FOUND_WORD;
        }
        Word wordInfo = *relevantWord;

        if (!wordInfo.partsOfSpeach.contains("Noun"))
        {
            return HandleWordStatus::NO_SPEACH_PART;
        }

        auto reading = conv_utf8_to_utf32(*wordInfo.readings.begin());
        std::optional<char32_t> lastKanaOpt = getLastKana(reading);
        std::optional<char32_t> firstKana = getFirstKana(reading);
        if (!lastKanaOpt)
        {
            return HandleWordStatus::GOT_ERROR;
        }
        if (!firstKana || ctx_.lastKana != toHiragana(*firstKana))
        {
            return HandleWordStatus::CANT_JOIN_WORDS;
        }

        char32_t lastKana = toHiragana(*lastKanaOpt);

        if (lastKana == U'ん')
        {
            stop_ = true;
            return HandleWordStatus::GOT_END_WORD;
        }

        // Now all filters were passed and words is good

        ctx_.lastPlayerId = id;
        ctx_.lastKana = lastKana;

        ps.score++;
        ctx_.wordsCount++;

        players_.erase(it);
        players_.insert(ps);

        words_.emplace_back(wordInfo);

        return HandleWordStatus::OK;
    }
}
