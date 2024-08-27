#include <cstdlib>

#include "logger.h"
#include "ini_config.h"
#include "http_connection.h"
#include <Windows.h>


void httpServer(tcp::acceptor& acceptor, tcp::socket& socket, IniConfig& conf)
{
    acceptor.async_accept(socket,
        [&](beast::error_code ec)
        {
            if (!ec)
                std::make_shared<HttpConnection>(std::move(socket), conf)->start();
            httpServer(acceptor, socket, conf);
        });
}


int main(int argc, char* argv[])
{
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);

    try
    {
        Logger::instance().setTag("[HttpServer]");
        IniConfig conf("config.ini");

        auto const address = net::ip::make_address("0.0.0.0");
        unsigned short port = conf.get<unsigned short>("server.port");

        net::io_context ioc{1};

        tcp::acceptor acceptor{ioc, { address, port }};
        tcp::socket socket{ioc};
        httpServer(acceptor, socket, conf);

        Logger::instance().log("Open browser and connect to http://localhost:"
                               + std::to_string(port) + " to see the web server operating");

        ioc.run();
    }
    catch (std::exception const& e)
    {
        Logger::instance().log(std::string("Error: ") + e.what());
        return EXIT_FAILURE;
    }
}
