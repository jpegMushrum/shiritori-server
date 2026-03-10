#include <sqlite3.h>
#include <stdexcept>

#include "user.hpp"
#include "users_repo.hpp"

using ull = unsigned long long;

UsersRepo::UsersRepo(std::string path) : dbPath_(path)
{
    initDb();
}

void UsersRepo::initDb()
{
    const std::string sql = R"(
            CREATE TABLE IF NOT EXISTS users (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                nickname TEXT NOT NULL,
                games INTEGER DEFAULT 0,
                words INTEGER DEFAULT 0
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

User UsersRepo::getUser(ull id)
{
    sqlite3* db;
    if (sqlite3_open(dbPath_.c_str(), &db) != SQLITE_OK)
        throw std::runtime_error("Cannot open DB");

    sqlite3_exec(db, "PRAGMA journal_mode=WAL;", nullptr, nullptr, nullptr);

    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, "SELECT id, nickname, games, words FROM users WHERE id=?;", -1, &stmt,
                       nullptr);
    sqlite3_bind_int(stmt, 1, id);

    User user;
    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        ull id = sqlite3_column_int(stmt, 0);
        std::string nickname = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        int games = sqlite3_column_int(stmt, 2);
        int words = sqlite3_column_int(stmt, 3);
        user = User(id, nickname, games, words);
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return user;
}

ull UsersRepo::addUser(User user)
{
    sqlite3* db;
    if (sqlite3_open(dbPath_.c_str(), &db) != SQLITE_OK)
        throw std::runtime_error("Cannot open DB");

    sqlite3_exec(db, "PRAGMA journal_mode=WAL;", nullptr, nullptr, nullptr);

    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, "INSERT INTO users (nickname) VALUES (?)", -1, &stmt, nullptr);

    sqlite3_bind_text(stmt, 1, user.nickname.c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) != SQLITE_DONE)
    {
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        throw std::runtime_error("Insert failed");
    }
    ull lastId = (ull)sqlite3_last_insert_rowid(db);

    sqlite3_finalize(stmt);
    sqlite3_close(db);

    return lastId;
}

void UsersRepo::changeUser(User user)
{
    sqlite3* db;
    if (sqlite3_open(dbPath_.c_str(), &db) != SQLITE_OK)
        throw std::runtime_error("Cannot open DB");

    sqlite3_exec(db, "PRAGMA journal_mode=WAL;", nullptr, nullptr, nullptr);

    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, "UPDATE users SET nickname = ?, games = ?, words = ? WHERE id = ?", -1,
                       &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, user.nickname.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, user.games);
    sqlite3_bind_int(stmt, 3, user.words);
    sqlite3_bind_int(stmt, 4, user.id);

    if (sqlite3_step(stmt) != SQLITE_DONE)
    {
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        throw std::runtime_error("Update failed");
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
}