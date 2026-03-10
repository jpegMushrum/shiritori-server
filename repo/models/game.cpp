#include "game.hpp"

using ull = unsigned long long;

Game::Game() : gameId(0), userId(0), words(0), place(0) {}

Game::Game(ull gameId, ull userId, int words, int place)
    : gameId(gameId), userId(userId), words(words), place(place)
{
}