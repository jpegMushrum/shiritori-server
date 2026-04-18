# Shiritori Game Server Protocol

## Overview

The Shiritori server communicates over TCP sockets using text-based commands and responses. All commands are space-separated strings terminated by a newline character.

> [!IMPORTANT]
> All requests must start with integer request id, and this id will be bounced back as 1-st argument in all responses. It doesn't apply anything to requests, it's needed for response selecting.

## Authentication

### login
**Usage:** `login <username>`

**Description:** Authenticate user and create a session. If the username doesn't exist in the database, a new user is automatically created.

**Response (Success):** Session ID string (32-character hexadecimal: 0-9, a-f)
```
Example: a1b2c3d4e5f6a7b8c9d0e1f2a3b4c5d6
```

**Errors:**
- `Error: login requires username` - Missing username argument
- `Error: invalid arguments for login` - Exception during processing

**Note:** All subsequent commands require the session ID for authentication.

---

### logout
**Usage:** `logout <sessionId>`

**Description:** Terminate the current session and invalidate the session ID. All subsequent commands using this session ID will fail with "Invalid session ID" error.

**Response (Success):** `Logged out successfully`

**Errors:**
- `Error: logout requires session ID` - Missing session ID argument
- `Error: Invalid session ID` - Session ID does not exist or already expired
- `Error: invalid arguments for logout` - Exception during processing

**Note:** This command is optional. Sessions automatically expire after 24 hours of creation even without explicit logout.

---
**Usage:** `addUser <nickname>`

**Description:** Create a new user with given nickname. This is a legacy command kept for backward compatibility.

**Response (Success):** User ID (unsigned long long integer)
```
Example: 12345
```

**Errors:**
- `Error: addUser requires nickname` - Missing nickname argument
- `Error: invalid arguments for addUser` - Exception during processing

---

## User Operations

### getUserInfo
**Usage:** `getUserInfo <sessionId>`

**Description:** Retrieve detailed information about the authenticated user.

**Response Format:** `<userId> <nickname> <averageWordsPerGame>`
```
Example: 42 JohnDoe 3.5
```

**Response Components:**
- `userId` - Unique user identifier (unsigned long long)
- `nickname` - Username string (no spaces)
- `averageWordsPerGame` - Float: total words Said / total games played (0 if no games)

**Errors:**
- `Error: getUserInfo requires session ID` - Missing session ID argument
- `Error: Invalid session ID` - Session ID does not exist or expired
- `Error: invalid arguments for getUserInfo` - Exception during processing

---

## Game Queries

### getActiveGames
**Usage:** `getActiveGames`

**Description:** List all currently active games.

**Response Format:** Game objects separated by semicolons (`;`)
```
Format: <gameId> <wordsCount> <playersCount> <adminId> <lastPlayerId> <lastKana>\n
Example: 10 5 2 1 2 し;20 3 1 5 0 あ
```

**Response Components (per game):**
- `gameId` - Unique game identifier
- `wordsCount` - Number of words said in the game
- `playersCount` - Number of players in the game
- `adminId` - User ID of the player who started the game
- `lastPlayerId` - User ID of the player who said the last word (0 if no words yet)
- `lastKana` - Last hiragana character (Japanese syllable for word continuation)

**Response (No Active Games):** Empty string

**Errors:**
- `Error: getActiveGames failed` - Exception during processing

---

### getGameInfo
**Usage:** `getGameInfo <gameId>`

**Description:** Retrieve detailed information about a specific game.

**Response Format:** `<gameId> <wordsCount> <playersCount> <adminId> <lastPlayerId> <lastKana>`
```
Example: 10 5 2 1 2 し
```

**Response Components:** (same as getActiveGames single game)

**Errors:**
- `Error: getGameInfo requires game ID` - Missing game ID argument
- `Error: invalid arguments for getGameInfo` - Invalid game ID format or exception

---

## Game Management

### startNewGame
**Usage:** `startNewGame <sessionId>`

**Description:** Create and start a new game session. The authenticated user becomes the admin (initiator).

**Response Format:** `<gameId> <wordsCount> <playersCount> <adminId> <lastPlayerId> <lastKana>`
```
Example: 100 0 1 42 0 あ
```

**Response Components:** (GameContext object for the newly created game)

**Errors:**
- `Error: startNewGame requires session ID` - Missing session ID argument
- `Error: Invalid session ID` - Session ID does not exist
- `Error: invalid arguments for startNewGame` - Exception during processing

---

### stopGame
**Usage:** `stopGame <sessionId> <gameId>`

**Description:** Stop and close an active game. Only the game admin can stop the game.

**Response (Success):** `Game stopped successfully`

**Errors:**
- `Error: stopGame requires session ID and game ID` - Missing arguments
- `Error: Invalid session ID` - Session ID does not exist
- `Error: invalid arguments for stopGame` - Invalid game ID format or exception

---

### addPlayerToGame
**Usage:** `addPlayerToGame <sessionId> <gameId>`

**Description:** Add the authenticated user to an existing game as a player. Returns the current game state (last kana and used words) and subscribes to game updates. After this query, the socket must become read-only.

**Response (Success):** `Player added successfully`

**Response (Game Join Info)** `playerJoinedGame <gameId> {"lastKana":string,"usedWords":[{"kanji":string,"meaning":string,"partsOfSpeach":[]string,"readings":[]string}]}`

**Response (Game Updates)** `gameUpdate <gameId> {event}`

Game Update Event Types:

1. **Word Played** (when a new word is accepted):
```json
{"type":"wordPlayed","word":{"kanji":string,"readings":[]string,"partsOfSpeach":[]string,"meaning":string},"lastKana":string}
```

2. **Game Stopped** (when game ends):
```json
{"type":"gameStopped","scores":{"userId":number,"score":number}}
```

**Errors:**
- `Error: addPlayerToGame requires session ID and game ID` - Missing arguments
- `Error: Invalid session ID` - Session ID does not exist
- `Error: invalid arguments for addPlayerToGame` - Invalid game ID format or exception

---

## Gameplay

### handleWord
**Usage:** `handleWord <gameId> <sessionId> <word>`

**Description:** Submit a word for the current game turn. The word must follow shiritori rules:
- Start with the hiragana sound that the previous word ended with
- Not have been said before in this game
- Be a valid Japanese word
- Be properly conjugated (match required speech part)

**Response Format:** Status code (one of the following)
```
OK                        - Word accepted
GOT_ERROR                 - Server error
WRONG_ORDER              - Word doesn't start with correct hiragana
NOT_JAPANESE_WORD        - Word not found in dictionary
NO_SPEACH_PART           - Word missing required speech part
NO_FOUND_WORD            - Word not in dictionary
GOT_END_WORD             - Word ends with ん (game-ending character)
GOT_DOUBLED_WORD         - Word was already said in this game
CANT_JOIN_WORDS          - Can't connect word to game flow
GAME_NOT_FOUND           - Game ID doesn't exist
GAME_STOPPED             - Game has been stopped
NO_FOUND_PLAYER          - Player not in this game
```

**Errors (Communication Level):**
- `Error: handleWord requires game ID, session ID, and word` - Missing arguments
- `Error: Invalid session ID` - Session ID does not exist
- `Error: invalid arguments for handleWord` - Invalid game ID format or exception

---

## Game History

### getGamesHistory
**Usage:** `getGamesHistory <sessionId>`

**Description:** Retrieve all past games for the authenticated user.

**Response Format:** Game history objects separated by semicolons (`;`)
```
Format: <gameId> <userId> <wordsCount> <place>\n
Example: 1 42 5 1;2 42 3 2;3 42 7 1
```

**Response Components (per game):**
- `gameId` - ID of the past game
- `userId` - User's ID (always matches authenticated user)
- `wordsCount` - Number of words the user said in that game
- `place` - Finishing position/rank (1 = first place)

**Response (No Games):** Empty string

**Errors:**
- `Error: getGamesHistory requires session ID` - Missing session ID argument
- `Error: Invalid session ID` - Session ID does not exist
- `Error: invalid arguments for getGamesHistory` - Exception during processing

---

## Error Handling

### General Error Responses

**Unknown Command:** `Error: Unknown command`
- Sent when command name doesn't match any of the above

**Invalid Arguments:** `Error: invalid arguments for <command>`
- Sent when argument parsing fails (e.g., non-numeric where int expected)

### Common Errors

| Error | Meaning |
|-------|---------|
| `Error: Invalid session ID` | Session doesn't exist (expired, never created, or invalid format) |
| `Error: <command> requires ...` | Missing required arguments |
| `Error: <command> failed` | Server-side error during processing |

---

## Command Sequences - Example Workflow

### 1. User Registration and Login
```
Client → Server: login alice
Server → Client: a1b2c3d4e5f6a7b8c9d0e1f2a3b4c5d6
```

### 2. Get User Information
```
Client → Server: getUserInfo a1b2c3d4e5f6a7b8c9d0e1f2a3b4c5d6
Server → Client: 1 alice 2.5
```

### 3. Start a New Game
```
Client → Server: startNewGame a1b2c3d4e5f6a7b8c9d0e1f2a3b4c5d6
Server → Client: 100 0 1 1 0 あ
```

### 4. Play a Word
```
Client → Server: handleWord 100 a1b2c3d4e5f6a7b8c9d0e1f2a3b4c5d6 あんず
Server → Client: OK
```

### 5. View Game History
```
Client → Server: getGamesHistory a1b2c3d4e5f6a7b8c9d0e1f2a3b4c5d6
Server → Client: 100 1 5 1
```

---

## Session Management

- **Session ID Format:** 32-character hexadecimal string
- **Session Lifetime:** Persists for the duration of the server process
- **Uniqueness:** Each login generates a unique session ID
- **Multiple Sessions:** One user can have multiple concurrent sessions
- **Session Requirements:** Required for all commands except `login` and `addUser`

---

## Session Management

### Session Lifetime (TTL)
- **Default TTL:** 24 hours (86400 seconds)
- **Expiration:** Sessions automatically expire 24 hours after creation
- **Cleanup:** Expired sessions are removed from memory:
  - During `login` (new session creation triggers cleanup)
  - During normal access (accessing expired session fails)
  - Automatic background cleanup ensures memory doesn't pile up

### Session Termination
- **Explicit:** Use `logout <sessionId>` to immediately invalidate a session
- **Implicit:** Sessions automatically expire after 24 hours without use
- **Multiple Sessions:** One user can have multiple concurrent active sessions (e.g., on different devices)
- **Memory Efficiency:** Expired sessions are periodically removed to prevent accumulation

### Session Access
- Each session access (via any command) checks:
  1. Session ID exists in storage
  2. Session has not expired (< 24 hours old)
  3. If either fails: "Error: Invalid session ID"

---

## Data Types

- **User ID / Game ID:** Unsigned long long (unsigned 64-bit integer)
- **String Fields:** No spaces allowed (space is command separator)
- **Numeric Fields:** Integers or floats as indicated
- **Japanese Text:** UTF-8 encoded hiragana characters
- **Delimiters:** Space (` `) separates fields within a line; semicolon (`;`) separates multiple objects

---

## Notes

1. All responses are space-delimited single-line strings unless multiple objects are present (use semicolon separator)
2. Session IDs are required for authenticated operations, but `login` and `addUser` work without a session
3. Game context includes hiragana lastKana - this is the character the next word must START with (for shiritori rules)
4. Empty responses (empty string) are valid for commands like `getActiveGames` and `getGamesHistory` when no data exists
5. The average words formula: totalWords ÷ totalGames (0.0 if user has never played)
6. Sessions automatically expire after 24 hours even without explicit logout - clients don't need to worry about infinite accumulation
