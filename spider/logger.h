#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include <string>
#include <boost/locale.hpp>

class Logger {
public:
    static Logger& instance() {
        if (!s_instance) {
            s_instance = new Logger();
        }
        return *s_instance;
    }

    void setTag(std::string&& tag) {
        m_tag = std::move(tag);
    }

    void log(std::string log) {
        std::cout << m_tag + ": " + log + "\n";
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
    static Logger* s_instance;
};

#endif // LOGGER_H
