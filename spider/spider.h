#ifndef SPIDER_H
#define SPIDER_H

#include "database_manager.h"
#include "link.h"

#include <mutex>
#include <queue>
#include <condition_variable>
#include <functional>



class Spider
{
public:
    Spider(DatabaseManager&&);
    bool startWithCompletion(Link link, int depth);


private:
    void parseLink(const Link& link, int depth);
    void threadPoolWorker();

private:
    DatabaseManager m_db;

    std::mutex mtx;
    std::condition_variable cv;
    std::queue<std::function<void()>> tasks;
    bool exitThreadPool = false;

};

#endif // SPIDER_H
