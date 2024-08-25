#include "spider.h"
#include "utils.h"


Spider::Spider(DatabaseManager& db) : m_db(db)
{
    m_db.dropTables();
    m_db.createTables();
}

bool Spider::startWithCompletion(Link link, int depth)
{
    Logger::instance().log("Spider::startWithCompletion");
    auto start = std::chrono::high_resolution_clock::now();

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

        Logger::instance().log("Spider has finished work");
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end - start;
        Logger::instance().log("Elapsed time: " + std::to_string(elapsed.count())
                               + " seconds. Indexed sites: " + std::to_string(indexedSites));

        return true;
    }
    catch (const std::exception& e)
    {
        Logger::instance().log(e.what());
        return false;
    }
}

void Spider::parseLink(const Link &link, int depth)
{
    try {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));

        Logger::instance().log("Processing link " + link.toString());
        std::string html = getHtmlContent(link);

        if (html.size() == 0)
        {
            Logger::instance().log("Failed to get HTML Content");
            return;
        }


        auto wordsCount = countWordFrequency(html);
        if (m_db.loadSiteIndex(wordsCount, link))
            indexedSites++;

        std::vector<Link> internalLinks = extract_links(html);
        std::vector<Link> links = get_new_unique_links(internalLinks, uniqueLinks);

        if (depth > 0) {
            std::lock_guard<std::mutex> lock(mtx);

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
        Logger::instance().log(e.what());
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
