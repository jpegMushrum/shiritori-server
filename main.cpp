#include <boost/asio.hpp>
#include <iostream>

using boost::asio::ip::tcp;

int main() {
    try {
        boost::asio::io_context io;

        tcp::acceptor acceptor(io, tcp::endpoint(tcp::v4(), 8080));

        std::cout << "Server started on port 8080...\n";

        while (true) {
            tcp::socket socket(io);
            acceptor.accept(socket);

            std::string message = "Hello from Boost server!\n";
            boost::asio::write(socket, boost::asio::buffer(message));
        }
    }
    catch (std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}