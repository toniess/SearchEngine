#include <iostream>
#include "ini_config.h"
#include "logger.h"
#include "spider.h"
#include <Windows.h>


int main()
{
    //SetConsoleOutputCP(1251);

    try {
        Logger::instance().setTag("[SpiderApp]");
        IniConfig conf("config.ini");
        Spider spider(conf);
        Link startLink = {  ProtocolType::HTTPS
                          , conf.get<std::string>("spider.start_point")
                          , conf.get<std::string>("spider.path")    };
        int depth = conf.get<int>("spider.depth");
        spider.startWithCompletion(startLink, depth);
    }
    catch (const std::string& e) {
        std::cout << e << std::endl;
    }
    catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
    }

    return 0;
}
