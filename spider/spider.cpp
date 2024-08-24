#include "spider.h"
#include "http_utils.h"


Spider::Spider(DatabaseManager&& db) : m_db(std::move(db))
{
    m_db.createTables();
}

bool Spider::startWithCompletion(Link link, int depth)
{
    Logger::instance().log("Spider::startWithCompletion");
    try {
        int numThreads = std::thread::hardware_concurrency();
        std::vector<std::thread> threadPool;

        for (int i = 0; i < numThreads; ++i) {
            threadPool.emplace_back(&Spider::threadPoolWorker, this);
        }

        {
            std::lock_guard<std::mutex> lock(mtx);
            tasks.push([link, this, depth]() { parseLink(link, depth); });
            cv.notify_one();
        }

        std::this_thread::sleep_for(std::chrono::seconds(2));

        {
            std::lock_guard<std::mutex> lock(mtx);
            exitThreadPool = true;
            cv.notify_all();
        }

        for (auto& t : threadPool) {
            t.join();
        }

        return true;
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << std::endl;
        return false;
    }
}

void Spider::parseLink(const Link &link, int depth)
{
    try {

        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        std::string html = getHtmlContent(link);

        if (html.size() == 0)
        {
            std::cout << "Failed to get HTML Content" << std::endl;
            return;
        }

        // TODO: Parse HTML code here on your own

        std::cout << "html content:" << std::endl;
        std::cout << html << std::endl;

        // TODO: Collect more links from HTML code and add them to the parser like that:

        std::vector<Link> links = {
                                   {ProtocolType::HTTPS, "en.wikipedia.org", "/wiki/Wikipedia"},
                                   {ProtocolType::HTTPS, "wikimediafoundation.org", "/"},
                                   };

        if (depth > 0) {
            std::lock_guard<std::mutex> lock(mtx);

            size_t count = links.size();
            size_t index = 0;
            for (auto& subLink : links)
            {
                tasks.push([subLink, depth, this]() { parseLink(subLink, depth - 1); });
            }
            cv.notify_one();
        }
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }

}

void Spider::threadPoolWorker() {
    std::unique_lock<std::mutex> lock(mtx);
    while (!exitThreadPool || !tasks.empty()) {
        if (tasks.empty()) {
            cv.wait(lock);
        }
        else {
            auto task = tasks.front();
            tasks.pop();
            lock.unlock();
            task();
            lock.lock();
        }
    }
}
