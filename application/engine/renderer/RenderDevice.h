#pragma once
/**
 * @file RenderDevice.h
 * @author Alan Abraham P Kochumon
 * @date Created on: September 24, 2026
 *
 * @brief Rendering device abstraction.
 *
 * @copyright Copyright (c) 2026 Alan Abraham P Kochumon
 */


// #define VULKAN_HPP_NO_EXCEPTIONS // TODO: Look into this before adding

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include "../platform/TempestSurface.h"
#include "RenderQueue.h"

#include <vector>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

namespace tempest::renderer
{
    class RenderDevice
    {
    public:
        explicit RenderDevice(const std::vector<const char*>& requiredExtensions) noexcept;

        [[nodiscard]] const vk::raii::PhysicalDevice& getPhysicalDevice() const noexcept { return _physicalDevice; }
        [[nodiscard]] const vk::raii::Device& getDevice() const noexcept { return _device; }
        [[nodiscard]] const std::vector<RenderQueue>& getQueues() const noexcept { return _queues; }
        [[nodiscard]] const vk::PhysicalDeviceFeatures& getDeviceFeatures() const noexcept { return _features; }

        [[nodiscard]] constexpr static uint32_t getMinAPIVersion() noexcept { return MIN_API_VERSION; }

        /// Get the maximum number of MSAA samples supported.
        [[nodiscard]] vk::SampleCountFlagBits getMaximumSupportSamples() const noexcept;

        /// Select a graphics device from the list of device installed on the platform.
        bool pickPhysicalDevice(const vk::raii::Instance& instance, uint32_t minAPIVersion = MIN_API_VERSION) noexcept;

        /// Create a logical device that interfaces with the physical device.
        bool createLogicalDevice(const platform::TempestSurface& surface, QueueConfig config) noexcept;

        /// TODO: Update for vk specific params to renderer ones
    protected:
    private:
        vk::raii::PhysicalDevice _physicalDevice;
        vk::raii::Device _device;
        std::vector<RenderQueue> _queues;
        vk::PhysicalDeviceFeatures _features;
        std::vector<const char*> _requiredExtensions;
        // TODO: Move to a top level location.
        static constexpr uint32_t MIN_API_VERSION{ vk::ApiVersion13 };
    };
} // namespace tempest::renderer