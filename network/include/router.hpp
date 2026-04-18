#pragma once

#include <functional>
#include <memory>
#include <string>

#include "game_context.hpp"
#include "game_info.hpp"
#include "game_update_event.hpp"
#include "handle_word_status.hpp"
#include "igame_session.hpp"
#include "igames_controller.hpp"
#include "iinfo_controller.hpp"
#include "player_score.hpp"
#include "session_manager.hpp"
#include "user_info.hpp"
#include "word_info.hpp"

class Router
{
  public:
    Router(std::function<void(std::string)>, std::shared_ptr<IInfoController>,
           std::shared_ptr<IGamesController>);

    void parseAndAnswer(std::string);

  private:
    std::function<void(std::string)> writeCb_;
    std::shared_ptr<IGamesController> gamesCtr_;
    std::shared_ptr<IInfoController> infoCtr_;
    static std::shared_ptr<SessionManager> sessionManager_;

    static std::string uiToString(const UserInfo&);
    static std::string gcToString(const GameContext&);
    static std::string giToString(const GameInfo&);
    static std::string statusToString(HandleWordStatus);
    static std::string wiToString(const WordInfo&);
    static std::string playerJoinInfoToString(const PlayerJoinInfo&);
    static std::string gameUpdateEventToString(const GameUpdateEvent&);
};
