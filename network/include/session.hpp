#pragma once

#include <boost/asio.hpp>
#include <istream>
#include <memory>

#include "igames_controller.hpp"
#include "iinfo_controller.hpp"
#include "router.hpp"

using tcp = boost::asio::ip::tcp;

class Session : public std::enable_shared_from_this<Session>
{
  public:
    Session(const Session&) = delete;
    Session& operator=(const Session&) = delete;
    Session(tcp::socket&&, std::shared_ptr<IInfoController>, std::shared_ptr<IGamesController>);

    void start();

  private:
    tcp::socket sock_;
    boost::asio::streambuf buffer_;

    std::shared_ptr<IInfoController> infoCtr_;
    std::shared_ptr<IGamesController> gamesCtr_;

    void doWrite(const std::string&);
    void doRead();
};
