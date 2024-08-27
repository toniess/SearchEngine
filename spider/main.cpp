#include "ini_config.h"
#include "logger.h"
#include "spider.h"
#include <Windows.h>


int main()
{
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);


    try {
        Logger::instance().setTag("[SpiderApp]");
        Logger::instance().setThreadLog(true);
        IniConfig conf("config.ini");
        DatabaseManager db(conf);
        Spider spider(db);
        Link startLink = {  ProtocolType::HTTPS
                          , conf.get<std::string>("spider.start_point")
                          , conf.get<std::string>("spider.path")    };
        int depth = conf.get<int>("spider.depth");
        spider.startWithCompletion(startLink, depth);
    }
    catch (const std::string& e) {
        Logger::instance().log(e);
    }
    catch (const std::exception& e) {
        Logger::instance().log(e.what());
    }

    return 0;
}
