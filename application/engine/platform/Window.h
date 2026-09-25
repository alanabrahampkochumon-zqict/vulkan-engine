#pragma once
/**
 * @file Window.h
 * @author Alan Abraham P Kochumon
 * @date Created on: September 23, 2026
 *
 * @brief Manages application windowing.
 *
 * @copyright Copyright (c) 2026 Alan Abraham P Kochumon
 */

#include <string>

/// Forward declaration
struct SDL_Window;

namespace tempest::platform
{
    class TempestWindow
    {
    public:
        TempestWindow(size_t width, size_t height, std::string windowName);
        bool initWindow() noexcept;

        ~TempestWindow() noexcept;

        TempestWindow(const TempestWindow& other)            = delete;
        TempestWindow& operator=(const TempestWindow& other) = delete;

        TempestWindow(TempestWindow&& other) noexcept;

        TempestWindow& operator=(TempestWindow&& other) noexcept;

        [[nodiscard]] constexpr SDL_Window* getCoreWindow() const noexcept;
        [[nodiscard]] constexpr size_t getWidth() const noexcept { return _width; }
        [[nodiscard]] constexpr size_t getHeight() const noexcept { return _height; }
        [[nodiscard]] constexpr std::string_view getWindowName() const noexcept { return _name; }


        static char const* const* getRequiredExtensions(uint32_t& count) noexcept;

    private:
        SDL_Window* _window;
        size_t _width, _height;
        std::string _name;
    };


    constexpr SDL_Window* TempestWindow::getCoreWindow() const noexcept { return _window; }
} // namespace tempest::platform