#pragma once
/**
 * @file Logger.h
 * @author Alan Abraham P Kochumon
 * @date Created on: September 25, 2026
 *
 * @brief API abstraction for logger.
 *
 * @copyright Copyright (c) 2026 Alan Abraham P Kochumon
 */

#include <string>

namespace tempest::log
{
    /// Initialize the logger. Must be called in the engine's init function.
    void init() noexcept;
    /// Shutdown the logger. Must be called in the engine's shutdown.
    void shutdown() noexcept;

    void info(std::string_view message) noexcept;
    void warn(std::string_view message) noexcept;
    void error(std::string_view message) noexcept;
} // namespace tempest::log