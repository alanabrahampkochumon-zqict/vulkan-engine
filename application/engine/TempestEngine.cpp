/**
 * @file TempestEngine.cpp
 * @author Alan Abraham P Kochumon
 * @date Created on: August 7, 2026
 *
 * @brief Implementation of declarations in VulkanRenderer.cppm
 *
 * @copyright Copyright (c) 2026 Alan Abraham P Kochumon
 */

module;
#include <format>
#include <print>
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS

#include <sdl3/SDL.h>
#include <sdl3/SDL_vulkan.h>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

module TempestEngine;


namespace engine
{
    void TempestEngine::init(const std::string& applicationName, const std::string& version, const std::string& id,
                             const size_t width, const size_t height)
    {
        appName      = applicationName;
        appVersion   = version;
        appId        = id;
        this->width  = width;
        this->height = height;

        SDL_SetAppMetadata(appName.c_str(), appVersion.c_str(), appId.c_str());
        if (!SDL_Init(SDL_INIT_VIDEO))
        {
            SDL_Log("Cannot initialize SDL window");
        }

        window = SDL_CreateWindow(appName.c_str(), this->width, this->height, SDL_WINDOW_VULKAN);

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
        initVulkan();
    }
    void TempestEngine::run() { std::println("Engine is running"); }
    void TempestEngine::cleanup() { SDL_DestroyWindow(window); }


    void TempestEngine::initVulkan()
    {
        const vk::ApplicationInfo applicationInfo{ .pApplicationName   = appName.c_str(),
                                                   .applicationVersion = VK_MAKE_VERSION(0, 0, 1),
                                                   .pEngineName        = ENGINE_NAME.c_str(),
                                                   .engineVersion      = VK_MAKE_VERSION(0, 0, 1),
                                                   .apiVersion         = vk::ApiVersion13 };


        uint32_t extensionCount{ 0 };
        const auto extensions = SDL_Vulkan_GetInstanceExtensions(&extensionCount);
        for (size_t i = 0; i < extensionCount; ++i)
        {
            const auto message = std::format("Extension {}: {}", i, extensions[i]);
            SDL_Log("%s", message.c_str());
        }
        const vk::InstanceCreateInfo instanceCreateInfo{
            .pApplicationInfo        = &applicationInfo,
            .enabledExtensionCount   = extensionCount,
            .ppEnabledExtensionNames = extensions,
        };

        instance = vk::raii::Instance(context, instanceCreateInfo);
    }
} // namespace engine
