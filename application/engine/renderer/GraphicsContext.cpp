/**
 * @file GraphicsContext.cpp
 * @author Alan Abraham P Kochumon
 * @date Created on: September 25, 2026
 *
 * @brief Implementation of member functions declared in GraphicsContext.h
 *
 * @copyright Copyright (c) 2026 Alan Abraham P Kochumon
 */

#include "GraphicsContext.h"

#include "../platform/Window.h"

namespace tempest::renderer
{
    GraphicsContext::GraphicsContext(const std::string& applicationName, const uint32_t appVersion,
                                     const std::string& engineName, const uint32_t engineVersion,
                                     const bool enableValidationLayers, const uint32_t minAPIVersion) noexcept
        : _enableValidationLayers(enableValidationLayers)
    { createVulkanInstance(applicationName, appVersion, engineName, engineVersion, minAPIVersion); }


    GraphicsContext::GraphicsContext(GraphicsContext&& other) noexcept
        : _context(std::move(other._context)),
          _instance(std::move(other._instance)),
          _debugMessenger(std::move(other._debugMessenger)),
          _enableValidationLayers(other._enableValidationLayers)
    {}


    GraphicsContext& GraphicsContext::operator=(GraphicsContext&& other) noexcept
    {
        if (this == &other)
            return *this;
        _context                = std::move(other._context);
        _instance               = std::move(other._instance);
        _debugMessenger         = std::move(other._debugMessenger);
        _enableValidationLayers = other._enableValidationLayers;
        return *this;
    }


    vk::Bool32 GraphicsContext::debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
                                              vk::DebugUtilsMessageTypeFlagsEXT type,
                                              const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                              void* pUserData)
    {
        // TODO:

        return vk::False;
    }

    bool GraphicsContext::createVulkanInstance(const std::string& applicationName, const uint32_t appVersion,
                                               const std::string& engineName, const uint32_t engineVersion,
                                               const uint32_t minAPIVersion) noexcept
    {
        const vk::ApplicationInfo applicationInfo{ .pApplicationName   = applicationName.c_str(),
                                                   .applicationVersion = VK_MAKE_VERSION(appVersion, 0, 0),
                                                   .pEngineName        = engineName.c_str(),
                                                   .engineVersion      = VK_MAKE_VERSION(engineVersion, 0, 0),
                                                   .apiVersion         = minAPIVersion };

        //------------
        // EXTENSIONS
        //------------
        const auto requiredExtensions  = getRequiredExtensions();
        const auto extensionProperties = _context.enumerateInstanceExtensionProperties();

        // Check if the extension we require are support by vulkan
        const auto unsupportedPropertiesIt =
            std::ranges::find_if(requiredExtensions, [&extensionProperties](const auto& requiredExtension) {
                return std::ranges::none_of(extensionProperties, [requiredExtension](const auto& extensionProperty) {
                    return strcmp(extensionProperty.extensionName, requiredExtension) == 0;
                });
            });
        if (unsupportedPropertiesIt != requiredExtensions.end())
        {
            return false;
        }



        //--------
        // LAYERS
        //--------
        std::vector<const char*> requiredLayers;
        if (_enableValidationLayers)
        {
            requiredLayers.assign(_validationLayers.begin(), _validationLayers.end());
        }
        // Enumerate through each layer and determine if the layers we need are supported
        const auto supportedLayers = _context.enumerateInstanceLayerProperties();
        const auto unsupportedLayers =
            std::ranges::find_if(requiredLayers, [&supportedLayers](const auto& requiredLayer) {
                return std::ranges::none_of(supportedLayers, [requiredLayer](const auto& layerProperty) {
                    return strcmp(layerProperty.layerName, requiredLayer) == 0;
                });
            });

        if (unsupportedLayers != requiredLayers.end())
        {
            return false;
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

        _instance = vk::raii::Instance(_context, instanceCreateInfo);
        return _instance != nullptr;
    }


    void GraphicsContext::setupDebugMessenger() noexcept
    {
        if (!_enableValidationLayers)
            return;

        constexpr vk::DebugUtilsMessageSeverityFlagsEXT severityFlags{
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eError | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning
        };
        constexpr vk::DebugUtilsMessageTypeFlagsEXT messageType{ vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
                                                                 vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                                                                 vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral };

        constexpr vk::DebugUtilsMessengerCreateInfoEXT debugUtilsCreateInfo{ .messageSeverity = severityFlags,
                                                                             .messageType     = messageType,
                                                                             .pfnUserCallback = &debugCallback };
        _debugMessenger = _instance.createDebugUtilsMessengerEXT(debugUtilsCreateInfo);
    }


    std::vector<const char*> GraphicsContext::getRequiredExtensions() const noexcept
    {
        uint32_t extensionCount     = 0;
        const auto windowExtensions = platform::TempestWindow::getRequiredExtensions(extensionCount);
        std::vector extensions(windowExtensions, windowExtensions + extensionCount);
        // Setup up the debug callback extension
        if (_enableValidationLayers)
            extensions.push_back(vk::EXTDebugUtilsExtensionName);
        return extensions;
    }



} // namespace tempest::renderer