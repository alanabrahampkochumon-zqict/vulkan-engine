#pragma once
/**
 * @file Application.h
 * @author Alan Abraham P Kochumon
 * @date Created on: September 23, 2026
 *
 * @brief Game engine application.
 *
 * @copyright Copyright (c) 2026 Alan Abraham P Kochumon
 */

#include "platform/Window.h"

namespace tempest
{
    using namespace platform;

    class TempestApp
    {
    public:
        explicit TempestApp(std::string name) noexcept;

        void run() noexcept;

        static constexpr size_t INIT_WIDTH  = 800;
        static constexpr size_t INIT_HEIGHT = 600;

    private:
        TempestWindow _window;
        bool _isRunning;

        void handleEvents() noexcept;
    };
} // namespace tempest