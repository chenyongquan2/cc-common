#include <boost/asio.hpp>
#include <map>
#include <mutex>

#include "cc-common/net.h"

struct io_context_relative {
    io_context_relative(size_t threads)
        : io_context_()
        , idle_work_(io_context_)
        , thread_pool_(threads)
    {
    }

    ~io_context_relative()
    {
        io_context_.stop();
        thread_pool_.join();
    }

    boost::asio::io_context io_context_;
    boost::asio::io_context::work idle_work_;
    boost::asio::thread_pool thread_pool_;
};

boost::asio::io_context& get_io_context(const std::string& name, size_t threads)
{
    static std::map<std::string, std::shared_ptr<io_context_relative>> io_context_manager;
    static std::mutex mutex;

    std::lock_guard<std::mutex> lock(mutex);
    auto iter = io_context_manager.find(name);
    if (iter != io_context_manager.end()) {
        return iter->second->io_context_;
    }

    io_context_manager[name] = std::make_shared<io_context_relative>(threads);
    for (size_t i = 0; i < threads; ++i) {
        boost::asio::post(
            io_context_manager[name]->thread_pool_, [&, name]() { io_context_manager[name]->io_context_.run(); });
    }
    return io_context_manager[name]->io_context_;
}
