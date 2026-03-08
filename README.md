# Shiritori Multithread Client and Server

> [!NOTE]
> Game rules on [wiki](https://en.wikipedia.org/wiki/Shiritori)

## User

Users play game, users have:

- Username
- History of games (place/points)
- Average poings per game
- Date of registration

## Game

Users write words, each word gives user a point. 
User can't write couple of words in row.
Game lasts while somebody doesn't writes a wrong word (ending with ん／ン), then his score being nullified.
Scores being stored, can start new game.

## Client

Client is an app, written with usage of QT. Client has 4 screens

1. Main screen
    1. Start new game
    2. Search for existing game
    3. Log in / Register
    4. Watch stats
2. Searching game screen
    1. Refresh
    2. Sort by players/used words count
    3. Filter by name
    4. Join game
    5. Back to main screen
3. Game screen
    1. List of used words
    2. Last kana
    3. Say new word
    4. Exit
4. Stats screen
    1. User info
    2. Back to main screen

## Server

Providing info about players.
Provides info about live games.
Handling several active games at once.

>[!NOTE]
>I wanted to Use [Japanese–Multilingual Dictionary](https://gitlab.com/yamagoya/jmdictdb) to verify words, but faced problems setting it up.
>I came up to use [jisho](jisho.org) open API.



1. UTF-8 Kana decoding
2. Comparing words 
3. Searching words through local jmdict server
4. Stroing and providing game info (also about already ended games)
5. Storing and providing profile info
