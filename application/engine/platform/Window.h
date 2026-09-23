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

#include <SDL3/SDL.h>
#include <string>

namespace tempest::platform
{
    class TempestWindow
    {
    public:
        TempestWindow(size_t width, size_t height, std::string windowName);
        bool initWindow() noexcept;

        ~TempestWindow() noexcept;



        [[nodiscard]] constexpr size_t getWidth() const noexcept { return _width; }
        [[nodiscard]] constexpr size_t getHeight() const noexcept { return _height; }
        [[nodiscard]] constexpr std::string_view getWindowName() const noexcept { return _name; }

    private:
        SDL_Window* _window;
        size_t _width, _height;
        std::string _name;
    };
} // namespace tempest::platform