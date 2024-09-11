#ifndef DATABASE_MANAGER_H
#define DATABASE_MANAGER_H

#include "ini_config.h"
#include <pqxx/pqxx>
#include "link.h"
#include "logger.h"


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

    bool loadSiteIndex(std::unordered_map<std::string, int> &m, const Link& l) {
        Logger::instance().log("DatabaseManager uploading data...");
        m_mutex.lock();
        try {
            pqxx::work w{ *m_connection };

            pqxx::result r = w.exec("INSERT INTO ref (host, path) VALUES (" +
                                     w.quote(l.hostName) + ", " + w.quote(l.query) + ") RETURNING id");
            int ref_id = r[0][0].as<int>();

            for (const auto& [word, count] : m) {
                w.exec("INSERT INTO data (reference_id, word, count) VALUES (" +
                       w.quote(ref_id) + ", " + w.quote(word) + ", " + w.quote(count) + ")");
            }

            w.commit();
            Logger::instance().log("DatabaseManager uploading data success");
            m_mutex.unlock();
            return true;
        } catch (const pqxx::sql_error& e) {
            Logger::instance().log(std::string("DatabaseManager uploading data failed. SQL error: ") + e.what()
                                   + std::string("Query was: ") + e.query());
        } catch (const std::exception& e) {
            Logger::instance().log(std::string("DatabaseManager uploading data failed. Error: ") + e.what());
        }
        m_mutex.unlock();
        return false;
    }

    std::vector<std::string> search(std::string query, int limit) {
        pqxx::work w{ *m_connection };

        std::vector<std::string> words = split_string(query, '+');

        for (auto& word : words) {
            for (auto& letter : word) {
                letter = std::tolower(letter);
            }
        }

        std::ostringstream sql;
        sql << "SELECT r.host, r.path, SUM(d.count) AS total_count "
            << "FROM ref r "
            << "JOIN data d ON r.id = d.reference_id "
            << "WHERE d.word IN (";

        for (size_t i = 0; i < words.size(); ++i) {
            if (i > 0) sql << ", ";
            sql << w.quote(words[i]);
        }
        sql << ") "
            << "GROUP BY r.host, r.path "
            << "ORDER BY total_count DESC "
            << "LIMIT "
            << limit;

        Logger::instance().log(std::string("processing query: ") + sql.str());
        pqxx::result result = w.exec(sql.str());
        w.commit();

        std::vector<std::string> results;
        for (const auto& row : result) {
            results.emplace_back(std::string("https://") + row[0].c_str() + std::string("/") + row[1].c_str());
            Logger::instance().log(results.back());
        }



        return results;
    }

    ~DatabaseManager() {
        m_connection->close();
        delete m_connection;
    }

private:

    std::vector<std::string> split_string(const std::string& s, char delimiter = ' ') {
        std::vector<std::string> words;
        std::istringstream stream(s);
        std::string word;
        while (std::getline(stream, word, delimiter)) {
            if (!word.empty()) {
                words.push_back(word);
            }
        }
        return words;
    }


private:
    pqxx::connection* m_connection;
    std::mutex m_mutex;

};

#endif // DATABASE_MANAGER_H
