#pragma once

#include <boost/asio/io_context.hpp>

boost::asio::io_context& get_io_context(
    // empty name for general global io_context
    // pass name to new/get a io_context for your own usage
    const std::string& name = "",
    // this variable only useful on initial call of {name} io_context
    size_t threads = std::thread::hardware_concurrency());
