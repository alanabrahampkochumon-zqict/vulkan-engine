/**
 * @file Application.cpp
 * @author Alan Abraham P Kochumon
 * @date Created on: September 23, 2026
 *
 * @brief Implementation of member functions declared in Application.h
 *
 * @copyright Copyright (c) 2026 Alan Abraham P Kochumon
 */

#include "Application.h"

#include "utils/Logger.h"

#include <SDL3/SDL.h>
#include <iostream>

namespace tempest
{

    TempestApp::TempestApp(std::string name, const uint32_t version) noexcept
        : _window{ INIT_WIDTH, INIT_HEIGHT, std::move(name) },
          _renderer{ _window, name, version, ENGINE_NAME, ENGINE_VERSION },
          _appName{ std::move(name) },
          _appVersion{ version },
          _isRunning{ false }
    {}


    bool TempestApp::init() noexcept
    {
        log::init();

        if (!_window.initWindow())
        {
            log::error("There was an error initializing the window!\n");
            return false;
        }

        _isRunning = true;
        return true;
    }

    void TempestApp::run() noexcept
    {
        log::info("App is running...");
        while (_isRunning)
        {
            handleEvents();
            // Update
            // Draw
        }
    }

    void TempestApp::handleEvents() noexcept
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
                case SDL_EVENT_QUIT:
                    _isRunning = false;
                    break;
                default:
                    break;
            }
        }
    }
} // namespace tempest