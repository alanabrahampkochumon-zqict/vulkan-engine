/**
 * @file VulkanEngine.cpp
 * @author Alan Abraham P Kochumon
 * @date Created on: August 7, 2026
 *
 * @brief Implementation of declarations in VulkanRenderer.cppm
 *
 * @copyright Copyright (c) 2026 Alan Abraham P Kochumon
 */

module;
#include <print>
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <sdl3/SDL.h>
#include <vulkan/vulkan.hpp>

module Engine;


namespace engine
{
    void TempestEngine::init()
    {
        // TODO: Move to parameter
        std::string appName    = "Tempest Game Engine";
        std::string appVersion = "0.0.1";
        std::string appId      = "com.tempest.engine";
        size_t width = 1280, height = 720;

        SDL_SetAppMetadata(appName.c_str(), appVersion.c_str(), appId.c_str());
        if (!SDL_Init(SDL_INIT_VIDEO))
        {
            SDL_Log("Cannot initialize SDL window");
        }

        auto window = SDL_CreateWindow(appName.c_str(), width, height, SDL_WINDOW_VULKAN);

        SDL_Event event;
        bool running = true;
        while (running)
        {
            while (SDL_PollEvent(&event))
            {
                switch (event.type)
                {
                    case SDL_EVENT_QUIT:
                        running = false;
                    default:
                        SDL_Log("Unhandled event!");
                }
            }
        }
    }
    void TempestEngine::run() { std::println("Engine is running"); }
    void TempestEngine::cleanup() { std::println("Engine is closing..."); }
} // namespace engine
