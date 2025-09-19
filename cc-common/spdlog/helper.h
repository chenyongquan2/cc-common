#pragma once

#include <string>
#include <memory>
#include <spdlog/spdlog.h>
#include <chrono>

struct spdlog_configuration {
    // you can specify the configuration file path for dynamically updating the log level
    // or any other feature that will be supported in the future
    /// attention: current only ini file was supported
    std::string dynamic_flush_configuration_path_;
    std::string log_level_key_ = "log.level";
    std::chrono::seconds periodic_update_interval_ = std::chrono::seconds(1);

    // log file will roll everyday and every roll_size bytes
    size_t roll_size_ = size_t(1024) * 1024 * 1024;
    bool remove_old_logs_ = true;
    size_t max_kept_files_ = 10;

    bool async_ = false;
    // this means log items, not total memory usage
    // assume that each log item is 1k, then 8192 means 8MB
    size_t async_buffer_size_ = 8192;
    size_t async_background_thread_count_ = 1;
};

void init_spdlog(const spdlog_configuration&, const std::string& file_prefix = "");