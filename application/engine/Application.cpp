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

#include <iostream>

namespace tempest
{

    TempestApp::TempestApp(std::string name) noexcept
        : _window{ INIT_WIDTH, INIT_HEIGHT, std::move(name) }, _isRunning(true) // TODO: Update to an init
    {
        // TODO: Move to init
        if (!_window.initWindow())
        {
            std::cout << "There was an error initializing the window!\n";
            return;
        }
    }

    void TempestApp::run() noexcept
    {
        std::cout << "App is running..." << '\n';
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