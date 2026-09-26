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

#include "../utils/Logger.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

namespace tempest::platform
{
    TempestWindow::TempestWindow(const size_t width, const size_t height, std::string windowName)
        : _window(nullptr), _width(width), _height(height), _name(std::move(windowName))
    {}

    bool TempestWindow::initWindow() noexcept
    {
        if (!SDL_Init(SDL_INIT_VIDEO))
        {
            log::error("There was an error instantiation SDL!");
            return false;
        }

        _window = SDL_CreateWindow(_name.c_str(), _width, _height, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);

        if (!_window)
        {
            log::error("There was an error creating the SDL window!");
            return false;
        }

        return true;
    }


    TempestWindow::TempestWindow(TempestWindow&& other) noexcept
        : _window(other._window), _width(other._width), _height(other._height)
    {}


    TempestWindow& TempestWindow::operator=(TempestWindow&& other) noexcept
    {
        if (this == &other)
            return *this;
        _window = other._window;
        _width  = other._width;
        _height = other._height;

        return *this;
    }

    char const* const* TempestWindow::getRequiredExtensions(uint32_t& count) noexcept
    { return SDL_Vulkan_GetInstanceExtensions(&count); }


    WindowExtent TempestWindow::getWindowExtent() const noexcept
    {
        int width{}, height{};
        SDL_GetWindowSizeInPixels(_window, &width, &height);
        return { .width = static_cast<size_t>(width), .height = static_cast<size_t>(height) };
    }


    TempestWindow::~TempestWindow() noexcept
    {
        SDL_DestroyWindow(_window);
        SDL_Quit();
    }


} // namespace tempest::platform