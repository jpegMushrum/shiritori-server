#include "game_fabric.hpp"

GameFabric::GameFabric(std::shared_ptr<IDictionary> dict, std::shared_ptr<GamesRepo> repo)
    : dict_(dict), repo_(repo)
{
}

std::shared_ptr<GameSession> GameFabric::createGame(ull gameId, ull adminId)
{
    return std::make_shared<GameSession>(gameId, adminId, dict_, repo_);
}