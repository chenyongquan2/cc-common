#pragma once

#include <fmt/format.h>
#include <boost/json.hpp>
#include <boost/asio.hpp>
#include <memory>
#include <chrono>
#include "cc-common/spdlog/helper.h"

#define THROW_EXCEPTION

void assert_file_exist(const std::string& filepath);

// block a thread forever
// eg: when you don't want main function returns
void endless_wait();

template <typename Callable>
void run_every(boost::asio::io_context& context, Callable callable, std::chrono::seconds interval)
{
    if (interval.count() == 0)
        return;

    // avoiding deadlock and equal to tick timer
    boost::asio::post(context, [callable]() { callable(); });

    auto timer = std::make_shared<boost::asio::steady_timer>(context);
    timer->expires_after(interval);
    timer->async_wait([&context, timer, interval, callable = std::forward<decltype(callable)>(callable)](
                          const boost::system::error_code& ec) {
        if (!ec) {
            run_every(context, callable, interval);
        }
    });
}

template <typename Callable>
void run_every(boost::asio::io_context& context, Callable callable, std::chrono::seconds interval,
    /* if you need to stop this routine, pass this value and set it in callable */
    std::shared_ptr<std::atomic_bool> stop)
{
    if (interval.count() == 0)
        return;

    if (not stop)
        stop = std::make_shared<std::atomic_bool>(false);

    if (stop->load())
        return;

    // avoiding deadlock and equal to tick timer
    boost::asio::post(context, [callable, stop]() { callable(stop); });

    auto timer = std::make_shared<boost::asio::steady_timer>(context);
    timer->expires_after(interval);
    timer->async_wait([&context, timer, interval, stop, callable = std::forward<decltype(callable)>(callable)](
                          const boost::system::error_code& ec) {
        if (!ec) {
            run_every(context, callable, interval, stop);
        }
    });
}
