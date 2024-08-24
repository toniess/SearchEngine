 #include <cstdlib>
#include <iostream>

 #include "http_connection.h"
#include <Windows.h>


 void httpServer(tcp::acceptor& acceptor, tcp::socket& socket)
 {
 	acceptor.async_accept(socket,
 		[&](beast::error_code ec)
 		{
 			if (!ec)
 				std::make_shared<HttpConnection>(std::move(socket))->start();
 			httpServer(acceptor, socket);
 		});
 }


int main(int argc, char* argv[])
{
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);

    try
    {

        auto const address = net::ip::make_address("0.0.0.0");
        unsigned short port = 8080;

        net::io_context ioc{1};

        tcp::acceptor acceptor{ioc, { address, port }};
        tcp::socket socket{ioc};
        httpServer(acceptor, socket);

        std::cout << "Open browser and connect to http://localhost:8080 to see the web server operating" << std::endl;

        ioc.run();
    }
    catch (std::exception const& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}
