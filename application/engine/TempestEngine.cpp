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

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS

#include <map>
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

        _isRunning = true;
    }


    void TempestEngine::run()
    {
        while (_isRunning)
        {
            handleEvents();
        }
    }


    void TempestEngine::cleanup() const
    {
        SDL_DestroyWindow(window);
        SDL_Quit();
    }


    void TempestEngine::initVulkan()
    {
        createVulkanInstance();
        setupDebugMessenger();
        createSurface();
        pickPhysicalDevice();
        createLogicalDevice();
    }


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


    void TempestEngine::setupDebugMessenger()
    {
        if (!enableValidationLayers)
            return;
        constexpr vk::DebugUtilsMessageSeverityFlagsEXT severityFlags{
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eError | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning
        };
        constexpr vk::DebugUtilsMessageTypeFlagsEXT messageType{ vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
                                                                 vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                                                                 vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral };

        vk::DebugUtilsMessengerCreateInfoEXT debugUtilsCreateInfo{ .messageSeverity = severityFlags,
                                                                   .messageType     = messageType,
                                                                   .pfnUserCallback = &debugCallback };
        debugMessenger = instance.createDebugUtilsMessengerEXT(debugUtilsCreateInfo);
    }


    void TempestEngine::createSurface()
    {
        VkSurfaceKHR surface;
        if (!SDL_Vulkan_CreateSurface(window, *instance, nullptr, &surface))
        {
            throw std::runtime_error("There was an error creating vulkan surface.");
        }
        this->surface = vk::raii::SurfaceKHR(instance, surface);
    }


    void TempestEngine::pickPhysicalDevice()
    {
        /// Properties represent the details about the device like name, vulkan version support etc.
        /// Features represent the feature-set supported by the device like certain shader support
        const auto physicalDevices = vk::raii::PhysicalDevices(instance);

        if (physicalDevices.empty())
            throw std::runtime_error("No Graphics card supporting vulkan found!");

        std::multimap<uint32_t, vk::raii::PhysicalDevice> gpus;

        for (const auto& pd : physicalDevices)
        {
            const auto properties = pd.getProperties();
            uint32_t score        = 0;

            // Support at least vulkan 1.3
            if (properties.apiVersion < vk::ApiVersion13)
                continue;

            // Must have graphics queue
            auto queueFamilies   = pd.getQueueFamilyProperties2();
            bool supportGraphics = std::ranges::any_of(queueFamilies, [](const auto& queueFamily) {
                return !!(queueFamily.queueFamilyProperties.queueFlags & vk::QueueFlagBits::eGraphics);
            });
            if (!supportGraphics)
                continue;

            // Must have required extensions
            std::vector requiredExtensions = { vk::KHRSwapchainExtensionName };
            const auto supportedExtensions = pd.enumerateDeviceExtensionProperties();
            bool supportsAllRequiredExtensions =
                std::ranges::all_of(requiredExtensions, [&supportedExtensions](const auto& requiredExtension) {
                    return std::ranges::any_of(
                        supportedExtensions, [requiredExtension](const auto& supportedExtension) {
                            return std::strcmp(supportedExtension.extensionName, requiredExtension);
                        });
                });
            if (!supportsAllRequiredExtensions)
                continue;

            // Must have some features like dynamic render
            const auto devFeatures = pd.getFeatures2<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan11Features,
                                                     vk::PhysicalDeviceVulkan13Features,
                                                     vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();
            const auto requiredFeatures = devFeatures.get<vk::PhysicalDeviceVulkan11Features>().shaderDrawParameters &&
                devFeatures.get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering &&
                devFeatures.get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState;
            if (!requiredFeatures)
                continue;


            // Discrete GPU is preferred
            if (properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
                score += 1000;


            // Sort by highest supported texture dimenension
            score += properties.limits.maxImageDimension3D;

            gpus.insert(std::make_pair(score, pd));
        }

        // If there is a gpu and score is greater than 0 for the last GPU (which gives the GPU with the highest score)
        // use that gpu
        if (!gpus.empty() && gpus.rbegin()->first > 0)
            physicalDevice = gpus.rbegin()->second;
        else
            throw std::runtime_error("No appropriate graphics card found!");
    }


    void TempestEngine::createLogicalDevice()
    {
        /// Request a device with graphics family queue
        /// and vulkan 1.1 shaderDrawparams, dynamic rendering and extended dynamic state
        /// Enable swap chain extension
        auto queueProperties = physicalDevice.getQueueFamilyProperties();

        uint32_t queueIndex = ~0; // 0b11111...1
        // Iterate through each queue and find the first one that supports both graphics and presentation
        for (uint32_t qFamilyIndex = 0; qFamilyIndex < queueProperties.size(); ++qFamilyIndex)
        {
            if (queueProperties[qFamilyIndex].queueFlags & vk::QueueFlagBits::eGraphics &&
                physicalDevice.getSurfaceSupportKHR(qFamilyIndex, *surface))
            {
                queueIndex = qFamilyIndex;
            }
        }
        if (queueIndex == ~0)
            throw std::runtime_error("Couldn't find a queue supporting both graphics and presentation");

        // We need to specify a priority even if we have only 1 queue
        float queuePriority = 0.5f;
        vk::DeviceQueueCreateInfo deviceQueueCreateInfo{ .queueFamilyIndex = queueIndex,
                                                         .queueCount       = 1,
                                                         .pQueuePriorities = &queuePriority };


        /// Vulkan Feature request chain
        /// Combine all the features required into a single struct without using pNext
        vk::StructureChain<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan11Features,
                           vk::PhysicalDeviceVulkan13Features, vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>
            featureChain = {
                {},                               // Vulkan 1.0 features
                { .shaderDrawParameters = true }, // Vulkan 1.1 features
                { .dynamicRendering = true },     // Vulkan 1.3 features
                { .extendedDynamicState = true }  // Dynamic state
            };

        std::vector requiredDeviceExtensions{ vk::KHRSwapchainExtensionName };
        vk::DeviceCreateInfo deviceCreateInfo{ .pNext                = &featureChain.get<vk::PhysicalDeviceFeatures2>(),
                                               .queueCreateInfoCount = 1,
                                               .pQueueCreateInfos    = &deviceQueueCreateInfo,
                                               .enabledExtensionCount =
                                                   static_cast<uint32_t>(requiredDeviceExtensions.size()),
                                               .ppEnabledExtensionNames = requiredDeviceExtensions.data() };

        device = vk::raii::Device(physicalDevice, deviceCreateInfo);

        // Queue is automatically created with logical device
        graphicsQueue = vk::raii::Queue(device, queueIndex, 0);
    }


    std::vector<const char*> TempestEngine::getRequiredExtensions() noexcept
    {
        uint32_t extensionCount  = 0;
        const auto sdlExtensions = SDL_Vulkan_GetInstanceExtensions(&extensionCount);
        std::vector extensions(sdlExtensions, sdlExtensions + extensionCount);
        // Setup up the debug callback extension
        if (enableValidationLayers)
            extensions.push_back(vk::EXTDebugUtilsExtensionName);
        return extensions;
    }


    VKAPI_ATTR vk::Bool32 VKAPI_CALL TempestEngine::debugCallback(
        const vk::DebugUtilsMessageSeverityFlagBitsEXT severity, const vk::DebugUtilsMessageTypeFlagsEXT type,
        const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
    {
        if (severity >= vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning)
        {
            SDL_Log(
                "%s",
                std::format("Validation Layer(type: {})\nMessage:\n{}\n", vk::to_string(type), pCallbackData->pMessage)
                    .c_str());
        }
        return vk::False;
    }


    void TempestEngine::handleEvents()
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
                    // SDL_Log("Unhandled event!");
            }
        }
    }
} // namespace engine
