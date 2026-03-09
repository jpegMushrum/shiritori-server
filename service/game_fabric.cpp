#include "game_fabric.hpp"

GameFabric::GameFabric(std::shared_ptr<IDictionary> dict) : dict_(dict) {}

std::shared_ptr<GameSession> GameFabric::createGame(ull gameId)
{
    return std::make_shared<GameSession>(gameId, dict_);
}