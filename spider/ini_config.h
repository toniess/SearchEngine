#ifndef INI_CONFIGH_H
#define INI_CONFIGH_H

#include <iostream>
#include <string>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

class IniConfig
{
public:
    IniConfig(std::string&& path)
    {
        try {
            boost::property_tree::ini_parser::read_ini(path, tree);
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }

    template<typename T>
    T get(std::string&& name) {
        try {
           return tree.get<T>(name);
        } catch (const std::exception& e) {
            std::cerr << "Warning: " << e.what() << std::endl;
            return T();
        }
    };

private:
    boost::property_tree::ptree tree;
};

#endif // INI_CONFIGH_H
