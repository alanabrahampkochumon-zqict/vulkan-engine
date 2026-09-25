/**
 * @file RenderDevice.cpp
 * @author Alan Abraham P Kochumon
 * @date Created on: September 24, 2026
 *
 * @brief Implementation of member function declared in RenderDevice.h
 *
 * @copyright Copyright (c) 2026 Alan Abraham P Kochumon
 */


#include "RenderDevice.h"

#include "../utils/Logger.h"

#include <map>

namespace tempest::renderer
{
    RenderDevice::RenderDevice(const std::vector<const char*>& requiredExtensions) noexcept
        : _physicalDevice{ nullptr }, _device{ nullptr }, _requiredExtensions(requiredExtensions)
    {}


    bool RenderDevice::pickPhysicalDevice(const vk::raii::Instance& instance, const uint32_t minAPIVersion) noexcept
    {
        /// Properties represent the details about the device like name, vulkan version support etc.
        /// Features represent the feature-set supported by the device like certain shader support
        const auto physicalDevices = vk::raii::PhysicalDevices(instance);

        if (physicalDevices.empty())
        {
            log::error("No Graphics card supporting vulkan found!");
        }

        std::multimap<uint32_t, vk::raii::PhysicalDevice> gpus;

        for (const auto& pd : physicalDevices)
        {
            const auto properties = pd.getProperties();
            uint32_t score        = 0;

            // Check for minimum API version
            if (properties.apiVersion < minAPIVersion)
                continue;

            // Must have graphics queue
            auto queueFamilies   = pd.getQueueFamilyProperties2();
            bool supportGraphics = std::ranges::any_of(queueFamilies, [](const auto& queueFamily) {
                return !!(queueFamily.queueFamilyProperties.queueFlags & vk::QueueFlagBits::eGraphics);
            });
            if (!supportGraphics)
                continue;

            // Must have required extensions
            const auto supportedExtensions = pd.enumerateDeviceExtensionProperties();
            bool supportsAllRequiredExtensions =
                std::ranges::all_of(_requiredExtensions, [&supportedExtensions](const auto& requiredExtension) {
                    return std::ranges::any_of(
                        supportedExtensions, [requiredExtension](const auto& supportedExtension) {
                            return std::strcmp(supportedExtension.extensionName, requiredExtension);
                        });
                });
            if (!supportsAllRequiredExtensions)
                continue;

            // TODO: Abstract these device features into a struct as well?
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
        {
            _physicalDevice = gpus.rbegin()->second;
            _features       = _physicalDevice.getFeatures();
            return true;
        }

        log::error("An error occurred while picking the physical device.");
        return false;
    }


    vk::SampleCountFlagBits RenderDevice::getMaximumSupportSamples() const noexcept
    {
        const vk::PhysicalDeviceProperties properties = _physicalDevice.getProperties();
        // Get the sample count for depth and color buffers
        const vk::SampleCountFlags count =
            properties.limits.framebufferColorSampleCounts & properties.limits.framebufferDepthSampleCounts;
        if (count & vk::SampleCountFlagBits::e64)
            return vk::SampleCountFlagBits::e64;
        if (count & vk::SampleCountFlagBits::e32)
            return vk::SampleCountFlagBits::e32;
        if (count & vk::SampleCountFlagBits::e16)
            return vk::SampleCountFlagBits::e16;
        if (count & vk::SampleCountFlagBits::e8)
            return vk::SampleCountFlagBits::e8;
        if (count & vk::SampleCountFlagBits::e4)
            return vk::SampleCountFlagBits::e4;
        if (count & vk::SampleCountFlagBits::e2)
            return vk::SampleCountFlagBits::e2;
        // Return 1 by default
        return vk::SampleCountFlagBits::e1;
    }

    bool RenderDevice::createLogicalDevice(const platform::TempestSurface& surface, const QueueConfig config) noexcept
    {
        /// Request a device with graphics family queue
        /// and vulkan 1.1 shaderDrawparams, dynamic rendering and extended dynamic state
        /// Enable swap chain extension
        std::vector<std::pair<vk::QueueFlagBits, RenderQueueType>> queueFlags;
        if (config.enableGraphicsQueue)
            queueFlags.push_back(std::make_pair(vk::QueueFlagBits::eGraphics, RenderQueueType::GRAPHICS));
        if (config.enableSeparateTransferQueue)
            queueFlags.push_back(std::make_pair(vk::QueueFlagBits::eTransfer, RenderQueueType::TRANSFER));
        if (config.enableComputeQueue)
            queueFlags.push_back(std::make_pair(vk::QueueFlagBits::eCompute, RenderQueueType::COMPUTE));

        _queues.resize(queueFlags.size());
        auto queueProperties = _physicalDevice.getQueueFamilyProperties();
        std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos(queueFlags.size());

        // Loop through each as per our config and create a queue
        for (size_t i = 0; i < queueFlags.size(); ++i)
        {
            // Iterate through each queue and find the first one that supports both graphics and presentation
            for (uint32_t qFamilyIndex = 0; qFamilyIndex < queueProperties.size(); ++qFamilyIndex)
            {
                if (queueProperties[qFamilyIndex].queueFlags & queueFlags[i].first &&
                    _physicalDevice.getSurfaceSupportKHR(qFamilyIndex, *surface.getSurface()))
                {
                    _queues[i].familyIndex = qFamilyIndex;
                    break;
                }
            }
            if (_queues[i].familyIndex == INVALID_QUEUE_INDEX)
                return false; // "Couldn't find a queue supporting both graphics and presentation"
            _queues[i].type = queueFlags[i].second;
            float queuePriority;
            queueCreateInfos[i] = { .queueFamilyIndex = _queues[i].familyIndex,
                                    .queueCount       = 1,
                                    .pQueuePriorities = &queuePriority };
        }

        /// TODO: Abstract request chain to enable user config.
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
                                               .queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
                                               .pQueueCreateInfos    = queueCreateInfos.data(),
                                               .enabledExtensionCount =
                                                   static_cast<uint32_t>(_requiredExtensions.size()),
                                               .ppEnabledExtensionNames = _requiredExtensions.data() };

        _device = vk::raii::Device(_physicalDevice, deviceCreateInfo);

        if (_device == nullptr)
            return false;

        // TODO: Add support multiple queues
        for (auto& queue : _queues)
        {
            queue.queue = vk::raii::Queue(_device, queue.familyIndex, 0);
            if (queue.queue == nullptr)
            {
                log::error("Couldn't create a render queue! Please check for graphics support.");
                return false;
            }
        }
        return true;
    }
} // namespace tempest::renderer