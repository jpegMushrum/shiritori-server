#include "server.hpp"

using tcp = boost::asio::ip::tcp;

Server::Server(boost::asio::io_context& ioContext, unsigned short port,
               std::shared_ptr<IInfoController> infoCtr, std::shared_ptr<IGamesController> gamesCtr)
    : ioContext_(ioContext), acceptor_(ioContext, tcp::endpoint(tcp::v4(), port)),
      infoCtr_(infoCtr), gamesCtr_(gamesCtr)
{
}

void Server::startAccepting()
{
    doAccept();
    ioContext_.run();
}

void Server::doAccept()
{
    acceptor_.async_accept(
        [this](boost::system::error_code ec, tcp::socket socket)
        {
            if (!ec)
            {
                std::make_shared<Session>(std::move(socket), infoCtr_, gamesCtr_)->start();
            }

            doAccept();
        });
}
