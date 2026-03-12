#include "game_fabric.hpp"

GameFabric::GameFabric(std::shared_ptr<IDictionary> dict, std::shared_ptr<IGamesRepo> repo)
    : dict_(dict), repo_(repo)
{
}

char32_t getRandomKana()
{
    static const std::vector<char32_t> allowedHiragana = {
        U'あ', U'い', U'う', U'え', U'お', U'か', U'き', U'く', U'け', U'こ', U'さ', U'し',
        U'す', U'せ', U'そ', U'た', U'ち', U'つ', U'て', U'と', U'な', U'に', U'ぬ', U'ね',
        U'の', U'は', U'ひ', U'ふ', U'へ', U'ほ', U'ま', U'み', U'む', U'め', U'も', U'や',
        U'ゆ', U'よ', U'ら', U'り', U'る', U'れ', U'ろ', U'わ', U'が', U'ぎ', U'ぐ', U'げ',
        U'ご', U'ざ', U'じ', U'ず', U'ぜ', U'ぞ', U'だ', U'で', U'ど', U'ば', U'び', U'ぶ',
        U'べ', U'ぼ', U'ぱ', U'ぴ', U'ぷ', U'ぺ', U'ぽ',
    };

    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(0, allowedHiragana.size() - 1);

    return allowedHiragana[dist(gen)];
}

std::shared_ptr<IGameSession> GameFabric::createGame(ull gameId, ull adminId)
{
    return std::make_shared<GameSession>(gameId, adminId, dict_, repo_, getRandomKana());
}
