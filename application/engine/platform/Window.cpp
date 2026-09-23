/**
 * @file Window.cpp
 * @author Alan Abraham P Kochumon
 * @date Created on: September 23, 2026
 *
 * @brief Platform window implementation declared in Window.h
 *
 * @copyright Copyright (c) 2026 Alan Abraham P Kochumon
 */

#include "Window.h"


namespace tempest::platform
{
    TempestWindow::TempestWindow(const size_t width, const size_t height, std::string windowName)
        : _window(nullptr), _width(width), _height(height), _name(std::move(windowName))
    {}

    bool TempestWindow::initWindow() noexcept
    {
        // TODO: Add logger
        if (!SDL_Init(SDL_INIT_VIDEO))
            return false;

        _window = SDL_CreateWindow(_name.c_str(), _width, _height, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);

        if (!_window)
            return false;

        return true;
    }


    TempestWindow::~TempestWindow() noexcept
    {
        SDL_DestroyWindow(_window);
        SDL_Quit();
    }

} // namespace tempest::platform