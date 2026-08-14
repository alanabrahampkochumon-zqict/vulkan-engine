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

        initVulkan();

        handleEvents();
    }


    void TempestEngine::run() { std::println("Engine is running"); }


    void TempestEngine::cleanup() const
    {
        SDL_DestroyWindow(window);
        SDL_Quit();
    }


    void TempestEngine::initVulkan() { createVulkanInstance(); }


    void TempestEngine::createVulkanInstance()
    {
        const vk::ApplicationInfo applicationInfo{ .pApplicationName   = appName.c_str(),
                                                   .applicationVersion = VK_MAKE_VERSION(0, 0, 1),
                                                   .pEngineName        = ENGINE_NAME.c_str(),
                                                   .engineVersion      = VK_MAKE_VERSION(0, 0, 1),
                                                   .apiVersion         = vk::ApiVersion13 };


        uint32_t extensionCount{ 0 };
        const auto requiredExtensions  = SDL_Vulkan_GetInstanceExtensions(&extensionCount);
        const auto extensionProperties = context.enumerateInstanceExtensionProperties();

        // Check if the extension we require are support by vulkan
        for (uint32_t i = 0; i < extensionCount; ++i)
        {
            if (std::ranges::none_of(extensionProperties,
                                     [sdlExtension = requiredExtensions[i]](const auto& extensionProperty) {
                                         return strcmp(extensionProperty.extensionName, sdlExtension) == 0;
                                     }))
            {
                throw std::runtime_error("Required SDL extension not supported:" + std::string(requiredExtensions[i]));
            }

            const auto message = std::format("Extension {}: {}", i, requiredExtensions[i]);
            SDL_Log("%s", message.c_str());
        }


        const vk::InstanceCreateInfo instanceCreateInfo{
            .pApplicationInfo        = &applicationInfo,
            .enabledExtensionCount   = extensionCount,
            .ppEnabledExtensionNames = requiredExtensions,
        };

        instance = vk::raii::Instance(context, instanceCreateInfo);
    }


    void TempestEngine::handleEvents()
    {
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
                        break;
                    default:
                        break;
                        // SDL_Log("Unhandled event!");
                }
            }
        }
    }
} // namespace engine
