#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include <map>
#include <mutex>
#include <string>
#include <boost/locale.hpp>

class Logger {
public:
    static Logger& instance() {
        static Logger instance;
        return instance;
    }

    void setTag(std::string&& tag) {
        m_tag = std::move(tag);
    }

    void log(std::string log) {
        logMutex.lock();
        system("cls");
        threadLogs[std::this_thread::get_id()] = m_tag + ": " + log + "\n";
        for (auto&& [threadId, log] : threadLogs) {
            std::cout << "thread_" << threadId << " " + log;
        }
        logMutex.unlock();
    }

private:
    Logger() {
        // boost::locale::generator gen;
        // std::locale loc = gen.generate("");
        // std::locale::global(loc);
        // std::wcout.imbue(loc);
    }

private:
    std::string m_tag;
    std::mutex logMutex;
    std::map<std::thread::id, std::string> threadLogs;
};

#endif // LOGGER_H
