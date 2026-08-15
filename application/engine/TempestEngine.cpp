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
    }


    void TempestEngine::run()
    {
        std::println("Engine is running");
        handleEvents();
    }


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

        //------------
        // EXTENSIONS
        //------------
        const auto requiredExtensions  = getRequiredExtensions();
        const auto extensionProperties = context.enumerateInstanceExtensionProperties();

        // Check if the extension we require are support by vulkan
        auto unsupportedPropertiesIt =
            std::ranges::find_if(requiredExtensions, [&extensionProperties](const auto& requiredExtension) {
                return std::ranges::none_of(extensionProperties, [requiredExtension](const auto& extensionProperty) {
                    return strcmp(extensionProperty.extensionName, requiredExtension) == 0;
                });
            });
        if (unsupportedPropertiesIt != requiredExtensions.end())
        {
            throw std::runtime_error(std::format("Required extension not supported! {}", *unsupportedPropertiesIt));
        }


        //--------
        // LAYERS
        //--------
        std::vector<const char*> requiredLayers;
        if (enableValidationLayers)
        {
            requiredLayers.assign(validationLayers.begin(), validationLayers.end());
        }
        // Enumerate through each layer and determine if the layers we need are supported
        const auto supportedLayers = context.enumerateInstanceLayerProperties();
        const auto unsupportedLayers =
            std::ranges::find_if(requiredLayers, [&supportedLayers](const auto& requiredLayer) {
                return std::ranges::none_of(supportedLayers, [requiredLayer](const auto& layerProperty) {
                    return strcmp(layerProperty.layerName, requiredLayer) == 0;
                });
            });

        if (unsupportedLayers != requiredLayers.end())
        {
            throw std::runtime_error("Required layer not supported: " + std::string(*unsupportedLayers));
        }


        //-------------------
        // INSTANCE CREATION
        //-------------------
        const vk::InstanceCreateInfo instanceCreateInfo{
            .pApplicationInfo        = &applicationInfo,
            .enabledLayerCount       = static_cast<uint32_t>(requiredLayers.size()),
            .ppEnabledLayerNames     = requiredLayers.data(),
            .enabledExtensionCount   = static_cast<uint32_t>(requiredExtensions.size()),
            .ppEnabledExtensionNames = requiredExtensions.data(),
        };

        instance = vk::raii::Instance(context, instanceCreateInfo);
    }


    std::vector<const char*> TempestEngine::getRequiredExtensions() noexcept
    {
        uint32_t extensionCount = 0;
        auto sdlExtensions      = SDL_Vulkan_GetInstanceExtensions(&extensionCount);
        std::vector extension(sdlExtensions, sdlExtensions + extensionCount);
        return extension;
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
