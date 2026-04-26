// Copyright(c) 2022-2025, Salvatore Cuzzilla (Swisscom AG)
// Copyright(c) 2026-present, Salvatore Cuzzilla (Avaloq, an NEC Company)
// Distributed under the MIT License (http://opensource.org/licenses/MIT)


#ifndef _CFG_TEST_COMMON_H_
#define _CFG_TEST_COMMON_H_

#include "utils/cfg_handler.h"

#include <gtest/gtest.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/null_sink.h>

#include <cstdio>
#include <map>
#include <memory>
#include <string>
#include <unistd.h>


namespace mdt_test {

// Registers null sinks for multi-logger and multi-logger-boot if absent.
// Idempotent — gtest fixtures share the spdlog registry across cases.
inline void EnsureNullLoggers() {
    auto null_sink = std::make_shared<spdlog::sinks::null_sink_mt>();
    if (!spdlog::get("multi-logger-boot")) {
        spdlog::register_logger(std::make_shared<spdlog::logger>(
            "multi-logger-boot", null_sink));
    }
    if (!spdlog::get("multi-logger")) {
        spdlog::register_logger(std::make_shared<spdlog::logger>(
            "multi-logger", null_sink));
    }
}

// RAII tmp cfg file. Body is written to a unique /tmp path; the file is
// removed on destruction. Path() returns the on-disk path for parsing.
class TmpCfg {
public:
    explicit TmpCfg(const std::string &body) {
        char path[] = "/tmp/mdt_ut_cfg_XXXXXX";
        int fd = mkstemp(path);
        EXPECT_GE(fd, 0);
        auto n = ::write(fd, body.data(), body.size());
        (void)n;
        ::close(fd);
        path_ = path;
    }
    ~TmpCfg() { std::remove(path_.c_str()); }
    const std::string &path() const { return path_; }
private:
    std::string path_;
};

}  // namespace mdt_test

#endif
