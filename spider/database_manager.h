#ifndef DATABASE_MANAGER_H
#define DATABASE_MANAGER_H

#include "ini_config.h"
#include <pqxx/pqxx>
#include "logger.h";

class DatabaseManager
{
public:
    DatabaseManager(IniConfig conf) {
        Logger::instance().log("DatabaseManager new connection");
        m_connection = new pqxx::connection("host=" + conf.get<std::string>("db.host")
                                       + " port=" + std::to_string(conf.get<int>("db.port"))
                                       + " dbname=" + conf.get<std::string>("db.name")
                                       + " user=" + conf.get<std::string>("db.user")
                                       + " password=" + conf.get<std::string>("db.password"));
        Logger::instance().log("DatabaseManager new connection success");
    }

    void dropTables() {
        Logger::instance().log("DatabaseManager dropping tables...");
        pqxx::work w{ *m_connection };

        w.exec("DROP TABLE IF EXISTS data");
        w.exec("DROP TABLE IF EXISTS ref");
        w.commit();
        Logger::instance().log("DatabaseManager drop tables success");
    }

    void createTables() {
        Logger::instance().log("DatabaseManager creating tables...");
        pqxx::work w{ *m_connection };

        w.exec("CREATE TABLE IF NOT EXISTS ref ("
               "id SERIAL PRIMARY KEY NOT NULL, "
               "host VARCHAR, "
               "path VARCHAR)");

        w.exec("CREATE TABLE IF NOT EXISTS data ("
               "id SERIAL PRIMARY KEY NOT NULL, "
               "reference_id INT NOT NULL REFERENCES ref (id), "
               "word VARCHAR, "
               "count INT)");

        w.commit();
        Logger::instance().log("DatabaseManager create tables success");
    }

private:
    pqxx::connection* m_connection;

};


#endif // DATABASE_MANAGER_H
