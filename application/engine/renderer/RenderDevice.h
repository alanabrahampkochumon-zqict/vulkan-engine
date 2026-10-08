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
#include "GraphicsContext.h"
#include "RenderQueue.h"
#include "TempestSurface.h"

#include <functional>
#include <vector>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

namespace tempest::renderer
{
    class RenderDevice
    {
    public:
        explicit RenderDevice(const GraphicsContext& context, const TempestSurface& surface) noexcept;
        RenderDevice(const RenderDevice& other)                = delete;
        RenderDevice& operator=(const RenderDevice& other)     = delete;
        RenderDevice& operator=(RenderDevice&& other) noexcept = delete;

        RenderDevice(RenderDevice&& other) noexcept;

        bool init(const std::vector<const char*>& requiredExtensions, const QueueConfig& config,
                  uint32_t minAPIVersion) noexcept;

        [[nodiscard]] const vk::raii::PhysicalDevice& getPhysicalDevice() const noexcept { return _physicalDevice; }
        [[nodiscard]] const vk::raii::Device& getBaseDevice() const noexcept { return _device; }
        [[nodiscard]] const std::vector<RenderQueue>& getQueues() const noexcept { return _queues; }
        [[nodiscard]] std::vector<RenderQueue>& getQueues() noexcept { return _queues; }
        [[nodiscard]] const vk::PhysicalDeviceFeatures& getDeviceFeatures() const noexcept { return _features; }

        /// Get the index of the memory type that supports the given properties.
        /// @return Index of the memory type if supported.
        /// @return UINT32_MAX otherwise.
        [[nodiscard]] uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) const noexcept;

        /// Get the maximum number of MSAA samples supported.
        [[nodiscard]] vk::SampleCountFlagBits getMaximumSupportSamples() const noexcept;

        /// Select a graphics device from the list of device installed on the platform.
        bool pickPhysicalDevice(const vk::raii::Instance& instance, const std::vector<const char*>& requiredExtensions,
                                uint32_t minAPIVersion) noexcept;

        /// Create a logical device that interfaces with the physical device.
        bool createLogicalDevice(const TempestSurface& surface, const std::vector<const char*>& requiredExtensions,
                                 QueueConfig config) noexcept;

        /// TODO: Update for vk specific params to renderer ones
    private:
        const GraphicsContext& _context;
        const TempestSurface& _surface;

        vk::raii::PhysicalDevice _physicalDevice;
        vk::raii::Device _device;
        std::vector<RenderQueue> _queues;
        vk::PhysicalDeviceFeatures _features;
    };
} // namespace tempest::renderer