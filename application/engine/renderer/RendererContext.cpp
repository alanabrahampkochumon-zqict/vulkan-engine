module;
/**
 * @file RendererContext.cpp
 * @author Alan Abraham P Kochumon
 * @date Created on: September 19, 2026
 *
 * @brief Implementation of member functions declared in RenderingContext.cppm.
 *
 * @copyright Copyright (c) 2026 Alan Abraham P Kochumon
 */

#include <format>
#include <map>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

module TempestEngine:Renderer;

namespace tempest
{
    constexpr RenderingContext::RenderingContext(std::string applicationName, const uint32_t applicationVersion,
                                                 const std::vector<const char*>& extensions,
                                                 const bool enableValidationLayers, std::string engineName,
                                                 const uint32_t engineVersion) noexcept
        : _engineName{ std::move(engineName) },
          _applicationName{ std::move(applicationName) },
          _appVersion{ applicationVersion },
          _engineVersion{ engineVersion },
          _enableValidationLayers(enableValidationLayers),
          _extensions{ extensions }
    {
        instantiateVkInstance();
        pickPhysicalDevice();
        createLogicalDevice();
    }


    constexpr const vk::raii::Instance& RenderingContext::getVulkanInstance() const noexcept { return _vulkanInstance; }

    // constexpr const vk::raii::Context& RenderingContext::getVulkanContext() const noexcept { return _vulkanContext; }

    ///+=+=+=+=+=+=+=+=+=+=+=
    ///  PRIVATE FUNCTIONS
    ///+=+=+=+=+=+=+=+=+=+=+=


    constexpr bool RenderingContext::instantiateVkInstance() noexcept
    {
        const vk::ApplicationInfo applicationInfo{ .pApplicationName   = _applicationName.c_str(),
                                                   .applicationVersion = VK_MAKE_VERSION(0, 0, 1),
                                                   .pEngineName        = _engineName.c_str(),
                                                   .engineVersion      = VK_MAKE_VERSION(0, 0, 1),
                                                   .apiVersion         = vk::ApiVersion13 };

        //------------
        // EXTENSIONS
        //------------
        if (_enableValidationLayers)
            _extensions.push_back(vk::EXTDebugUtilsExtensionName);

        const auto extensionProperties = _vulkanContext.enumerateInstanceExtensionProperties();

        // Check if the extension we require are support by vulkan
        const auto unsupportedPropertiesIt =
            std::ranges::find_if(_extensions, [&extensionProperties](const auto& requiredExtension) {
                return std::ranges::none_of(extensionProperties, [requiredExtension](const auto& extensionProperty) {
                    return strcmp(extensionProperty.extensionName, requiredExtension) == 0;
                });
            });
        if (unsupportedPropertiesIt != _extensions.end())
        {
            _log(std::format("Extension support not found!\n{}", std::string(*unsupportedPropertiesIt)));
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
        const auto supportedLayers = _vulkanContext.enumerateInstanceLayerProperties();
        const auto unsupportedLayers =
            std::ranges::find_if(requiredLayers, [&supportedLayers](const auto& requiredLayer) {
                return std::ranges::none_of(supportedLayers, [requiredLayer](const auto& layerProperty) {
                    return strcmp(layerProperty.layerName, requiredLayer) == 0;
                });
            });

        if (unsupportedLayers != requiredLayers.end())
        {
            _log(std::format("Layer support not found!\n{}", std::string(*unsupportedLayers)).c_str());
            return false;
        }


        //-------------------
        // INSTANCE CREATION
        //-------------------
        const vk::InstanceCreateInfo instanceCreateInfo{
            .pApplicationInfo        = &applicationInfo,
            .enabledLayerCount       = static_cast<uint32_t>(requiredLayers.size()),
            .ppEnabledLayerNames     = requiredLayers.data(),
            .enabledExtensionCount   = static_cast<uint32_t>(_extensions.size()),
            .ppEnabledExtensionNames = _extensions.data(),
        };

        _vulkanInstance = vk::raii::Instance(_vulkanContext, instanceCreateInfo);

        return true;
    }


    constexpr bool RenderingContext::pickPhysicalDevice() noexcept
    {
        /// Query the available physical devices.
        const auto physicalDevices = vk::raii::PhysicalDevices(_vulkanInstance);

        if (physicalDevices.empty())
        {
            _log("No Graphics card supporting vulkan found!");
            return false;
        }


        /// Sort them by the properties and features we require.
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
            const auto requiredFeatures = devFeatures.get<vk::PhysicalDeviceFeatures2>().features.samplerAnisotropy &&
                devFeatures.get<vk::PhysicalDeviceVulkan11Features>().shaderDrawParameters &&
                devFeatures.get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering &&
                devFeatures.get<vk::PhysicalDeviceVulkan13Features>().synchronization2 &&
                devFeatures.get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState;
            if (!requiredFeatures)
                continue;


            // Discrete GPU is preferred
            if (properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
                score += 1000;


            // Sort by highest supported texture dimension
            score += properties.limits.maxImageDimension3D;

            gpus.insert(std::make_pair(score, pd));
        }
        // If there is a gpu and score is greater than 0 for the last GPU (which gives the GPU with the highest score)
        // use that gpu
        if (!gpus.empty() && gpus.rbegin()->first > 0)
            _selectedGPU = gpus.rbegin()->second;
        else
        {
            _log("No appropriate graphics card found!");
            return false;
        }

        // TODO: Add msaa samples
        // msaaSamples = getMaxUsableSampleCount();

        return true;
    }

    constexpr bool RenderingContext::createLogicalDevice() noexcept
    { /// Request a device with graphics family queue
        /// and vulkan 1.1 shaderDrawparams, dynamic rendering and extended dynamic state
        /// Enable swap chain extension
        auto queueProperties = _selectedGPU.getQueueFamilyProperties();

        // Iterate through each queue and find the first one that supports both graphics and presentation
        for (uint32_t qFamilyIndex = 0; qFamilyIndex < queueProperties.size(); ++qFamilyIndex)
        {
            if (queueProperties[qFamilyIndex].queueFlags & vk::QueueFlagBits::eGraphics &&
                _selectedGPU.getSurfaceSupportKHR(qFamilyIndex, *_surface))
            {
                queueIndex = qFamilyIndex;
                break;
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
                { .features = { .sampleRateShading = true, .samplerAnisotropy = true } }, // Physical Device Features
                { .shaderDrawParameters = true },                                         // Vulkan 1.1 features
                { .synchronization2 = true, .dynamicRendering = true },                   // Vulkan 1.3 features
                { .extendedDynamicState = true }                                          // Dynamic state
            };

        std::vector requiredDeviceExtensions{ vk::KHRSwapchainExtensionName };
        vk::DeviceCreateInfo deviceCreateInfo{ .pNext                = &featureChain.get<vk::PhysicalDeviceFeatures2>(),
                                               .queueCreateInfoCount = 1,
                                               .pQueueCreateInfos    = &deviceQueueCreateInfo,
                                               .enabledExtensionCount =
                                                   static_cast<uint32_t>(requiredDeviceExtensions.size()),
                                               .ppEnabledExtensionNames = requiredDeviceExtensions.data() };

        _logicalDevice = vk::raii::Device(_selectedGPU, deviceCreateInfo);

        // Queue is automatically created with logical device
        _graphicsQueue = vk::raii::Queue(_logicalDevice, queueIndex, 0);
    }


    void RenderingContext::attachLogger(const std::function<void(std::string)>& loggingFunc) noexcept
    { _log = loggingFunc; }
} // namespace tempest