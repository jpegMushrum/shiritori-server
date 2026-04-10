# Shiritori Multithread Server

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

## Protocol
TODO
