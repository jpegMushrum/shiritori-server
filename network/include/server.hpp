#pragma once

#include <boost/asio.hpp>
#include <iostream>
#include <memory>

#include "igames_controller.hpp"
#include "iinfo_controller.hpp"
#include "session.hpp"

using tcp = boost::asio::ip::tcp;

class Server : public std::enable_shared_from_this<Server>
{
  public:
    Server(boost::asio::io_context&, unsigned short port, std::shared_ptr<IInfoController>,
           std::shared_ptr<IGamesController>);

    void startAccepting();

  private:
    void doAccept();

    boost::asio::io_context& ioContext_;
    tcp::acceptor acceptor_;

    std::shared_ptr<IInfoController> infoCtr_;
    std::shared_ptr<IGamesController> gamesCtr_;
};
