#include "helper.h"
#include "cc-common/net.h"
#include "cc-common/utils.h"

#include <cstddef>
#include <fstream>
#include <chrono>
#include <boost/program_options.hpp>

#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/sinks/msvc_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include "spdlog/sinks/rotating_file_sink.h"
#include <spdlog/spdlog.h>
#include <spdlog/async.h>

namespace po = boost::program_options;

void periodic_update_log_level_from_ini(const std::string& ini_path, std::shared_ptr<spdlog::logger> logger,
    const std::string key_in_ini, std::chrono::seconds interval)
{
    run_every(
        get_io_context(),
        [=]() {
            if (not logger or key_in_ini.empty())
                return;

            std::ifstream ifs(ini_path);
            if (!ifs)
                return;

            // clang-format off
            static std::unordered_map<std::string, spdlog::level::level_enum> um { 
                {"trace"    , spdlog::level::trace,   },
                {"debug"    , spdlog::level::debug,   },
                {"info"     , spdlog::level::info,    },
                {"warn"     , spdlog::level::warn,    },
                {"error"    , spdlog::level::err,     },
                {"critical" , spdlog::level::critical },
            };
            // clang-format on

            po::options_description option;
            po::variables_map vm;
            std::string level;
            option.add_options()(key_in_ini.c_str(), po::value<std::string>(&level)->default_value("info"));
            po::store(po::parse_config_file(ifs, option, true), vm);
            po::notify(vm);

            if (um.find(level) == um.end()) {
                SPDLOG_ERROR("invalid logger level:{}", level);
                return;
            }

            logger->set_level(um[level]);
        },
        std::chrono::seconds(interval));
}

void init_spdlog(const spdlog_configuration& c, const std::string& file_prefix)
{
    std::vector<spdlog::sink_ptr> sinks;

#ifdef _DEBUG
    // for vscode debug console
    sinks.emplace_back(std::make_shared<spdlog::sinks::msvc_sink_mt>());
    sinks.emplace_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
#endif
    auto max_size = 500 * 1024 * 1024;
    auto max_files = 5;
    // using 20 as prefix is that devops use regex 20.* to grep logs need to be uploaded to victoria log
    std::string logfile = "RunLog/" + (file_prefix.empty() ? std::string("") : file_prefix + "-") + "20-rotation.log";
    auto sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(logfile, max_size, max_files);
    sinks.emplace_back(sink);

    if (c.async_) {
        spdlog::init_thread_pool(c.async_buffer_size_, c.async_background_thread_count_);
        auto logger = std::make_shared<spdlog::async_logger>(
            "async_logger", sinks.begin(), sinks.end(), spdlog::thread_pool(), spdlog::async_overflow_policy::block);
        spdlog::set_default_logger(logger);
    } else {
        auto logger = std::make_shared<spdlog::logger>("", sinks.begin(), sinks.end());
        spdlog::set_default_logger(logger);
    }

    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%t] [%s:%#] [%!] %v");
    spdlog::flush_every(std::chrono::seconds(1));

#ifdef _DEBUG
    spdlog::set_level(spdlog::level::debug);
#endif

    if (not c.dynamic_flush_configuration_path_.empty()) {
        periodic_update_log_level_from_ini(c.dynamic_flush_configuration_path_, spdlog::default_logger(),
            c.log_level_key_, c.periodic_update_interval_);
    }
}