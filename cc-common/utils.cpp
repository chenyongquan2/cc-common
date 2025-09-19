#include "utils.h"

#include <fstream>
#include <spdlog/spdlog.h>
#include <mutex>

bool file_exist(const std::string& filepath)
{
    std::ifstream file(filepath);
    if (not file.good()) {
        SPDLOG_WARN("file not exist:{}", filepath);
    }
    return file.good();
}

void assert_file_exist(const std::string& filepath)
{
    if (!file_exist(filepath)) {
        throw std::runtime_error("file not exist:" + filepath);
    }
}

void endless_wait()
{
    // 相当于实现while(true)的效果，但是这种写法可以让权等待，不会占用cpu核心
    std::condition_variable cv;
    std::mutex mu;
    std::unique_lock<std::mutex> lock(mu);
    cv.wait(lock, [] { return false; });
}