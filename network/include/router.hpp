#pragma once

#include <functional>
#include <memory>
#include <string>

#include "game_context.hpp"
#include "game_info.hpp"
#include "handle_word_status.hpp"
#include "igames_controller.hpp"
#include "iinfo_controller.hpp"
#include "user_info.hpp"

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

    static std::string uiToString(const UserInfo&);
    static std::string gcToString(const GameContext&);
    static std::string giToString(const GameInfo&);
    static std::string statusToString(HandleWordStatus);
};
