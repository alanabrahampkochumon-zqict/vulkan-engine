/**
 * @file Logger.cpp
 * @author Alan Abraham P Kochumon
 * @date Created on: September 25, 2026
 *
 * @brief Implementation of Logging function declared in Logger.h
 *
 * @copyright Copyright (c) 2026 Alan Abraham P Kochumon
 */


#include "Logger.h"

#include <memory>
#include <quill/LogFunctions.h>
#include <quill/SimpleSetup.h>

namespace tempest::log
{

    static std::shared_ptr<quill::Logger> _coreLogger;

    void init() noexcept
    {
        if (_coreLogger == nullptr)
            _coreLogger = std::shared_ptr<quill::Logger>(quill::simple_logger());
    }

    void shutdown() noexcept {}

    void info(const std::string_view message) noexcept { quill::info(_coreLogger.get(), message.data()); }

    void warn(const std::string_view message) noexcept { quill::warning(_coreLogger.get(), message.data()); }

    void error(const std::string_view message) noexcept { quill::error(_coreLogger.get(), message.data()); }
} // namespace tempest::log