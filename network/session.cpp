#include "session.hpp"

using tcp = boost::asio::ip::tcp;

Session::Session(tcp::socket&& sock, std::shared_ptr<IInfoController> infoCtr,
                 std::shared_ptr<IGamesController> gamesCtr)
    : sock_(std::move(sock)), infoCtr_(infoCtr), gamesCtr_(gamesCtr)
{
}

void Session::start()
{
    doRead();
}

void Session::doRead()
{
    auto self = shared_from_this();

    boost::asio::async_read_until(
        sock_, buffer_, "\n",
        [this, self](boost::system::error_code ec, std::size_t)
        {
            if (!ec)
            {
                std::istream is(&buffer_);
                std::string request;
                std::getline(is, request);

                std::function<void(std::string)> writeCb = [this, self](std::string response)
                {
                    boost::asio::post(sock_.get_executor(),
                                      [this, self, response]() { doWrite(response); });
                };

                Router router(writeCb, infoCtr_, gamesCtr_);
                router.parseAndAnswer(request);
            }
        });
}

void Session::doWrite(const std::string& response)
{
    auto self = shared_from_this();

    boost::asio::async_write(sock_, boost::asio::buffer(response + "\n"),
                             [this, self](boost::system::error_code ec, std::size_t)
                             {
                                 if (!ec)
                                 {
                                     doRead();
                                 }
                             });
}