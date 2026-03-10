#include "games_repo.hpp"

#include <sqlite3.h>
#include <stdexcept>

GamesRepo::GamesRepo(std::string path) : dbPath_(path)
{
    initTables();
}

void GamesRepo::initTables()
{
    const std::string sql = R"(
            CREATE TABLE IF NOT EXISTS games (
                id INTEGER PRIMARY KEY AUTOINCREMENT
            );
    

            CREATE TABLE IF NOT EXISTS game_history (
                game_id INTEGER NOT NULL,
                user_id INTEGER NOT NULL,
                words INTEGER DEFAULT 0,
                place INTEGER DEFAULT 0,
                PRIMARY KEY (game_id, user_id)
            );
        )";

    sqlite3* db;
    if (sqlite3_open(dbPath_.c_str(), &db) != SQLITE_OK)
        throw std::runtime_error("Cannot open DB");

    sqlite3_exec(db, "PRAGMA journal_mode=WAL;", nullptr, nullptr, nullptr);

    char* errMsg = nullptr;
    if (sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK)
    {
        std::string err = errMsg ? errMsg : "Unknown error";
        sqlite3_free(errMsg);
        sqlite3_close(db);
        throw std::runtime_error(err);
    }

    sqlite3_close(db);
}

void GamesRepo::putGame(std::vector<Game> gameParts)
{
    sqlite3* db;
    if (sqlite3_open(dbPath_.c_str(), &db) != SQLITE_OK)
        throw std::runtime_error("Cannot open DB");

    sqlite3_exec(db, "PRAGMA journal_mode=WAL;", nullptr, nullptr, nullptr);
    sqlite3_exec(db, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr);

    char* errMsg = nullptr;

    sqlite3_stmt* stmt;

    sqlite3_prepare_v2(db, "INSERT INTO games DEFAULT VALUES", -1, &stmt, nullptr);

    if (sqlite3_step(stmt) != SQLITE_DONE)
        throw std::runtime_error("Insert game failed");

    sqlite3_finalize(stmt);

    ull gameId = (ull)sqlite3_last_insert_rowid(db);

    sqlite3_prepare_v2(
        db, "INSERT INTO game_history (game_id, user_id, words, place) VALUES (?, ?, ?, ?)", -1,
        &stmt, nullptr);

    for (int i = 0; i < gameParts.size(); i++)
    {
        auto game = gameParts[i];

        sqlite3_bind_int64(stmt, 1, gameId);
        sqlite3_bind_int64(stmt, 2, game.userId);
        sqlite3_bind_int(stmt, 3, game.words);
        sqlite3_bind_int(stmt, 4, game.place);

        if (sqlite3_step(stmt) != SQLITE_DONE)
            throw std::runtime_error("Insert game_history failed");

        sqlite3_reset(stmt);
    }

    sqlite3_finalize(stmt);

    sqlite3_exec(db, "COMMIT;", nullptr, nullptr, nullptr);

    sqlite3_close(db);
}

std::vector<Game> GamesRepo::getGameHistoryByUserId(ull userId)
{
    sqlite3* db;
    if (sqlite3_open(dbPath_.c_str(), &db) != SQLITE_OK)
        throw std::runtime_error("Cannot open DB");

    sqlite3_exec(db, "PRAGMA journal_mode=WAL;", nullptr, nullptr, nullptr);

    const char* sql = R"(
        SELECT game_id, words, place
        FROM game_history
        WHERE user_id = ?
        )";

    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);

    sqlite3_bind_int64(stmt, 1, userId);

    std::vector<Game> games;
    Game currentGame;
    ull currentGameId = 0;

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        ull gameId = sqlite3_column_int64(stmt, 0);
        int words = sqlite3_column_int(stmt, 1);
        int place = sqlite3_column_int(stmt, 2);

        currentGame = Game();
        currentGame.userId = userId;
        currentGame.gameId = gameId;
        currentGame.words = words;
        currentGame.place = place;

        games.push_back(currentGame);
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);

    return games;
}