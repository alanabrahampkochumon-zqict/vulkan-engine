#pragma once
/**
 * @file Buffer.h
 * @author Alan Abraham P Kochumon
 * @date Created on: September 29, 2026
 *
 * @brief Abstraction for Buffers and Memory.
 *
 * @copyright Copyright (c) 2026 Alan Abraham P Kochumon
 */


#include "RenderDevice.h"

#include <vulkan/vulkan_raii.hpp>

namespace tempest::renderer
{
    class Buffer
    {
    public:
        // TODO: Update to custom flags
        Buffer(RenderDevice& device, size_t size, vk::BufferUsageFlagBits usageFlags,
               vk::MemoryPropertyFlags memProperties) noexcept;

        Buffer(const Buffer& other)            = delete;
        Buffer& operator=(const Buffer& other) = delete;

        /// Copy the current buffer to a new destinations.
        void copyTo(const Buffer& destination, size_t size, RenderQueue& queue) const noexcept;

        /// TODO: Add CopyToImage

        /// Copy @p source buffer to @p destination.
        static void copy(const Buffer& source, const Buffer& destination, size_t size,
                         RenderQueue& queue) noexcept;

        void memcpy(const void* source, size_t size) const noexcept;

        [[nodiscard]] const vk::raii::Buffer& getBaseBuffer() const { return _buffer; }
        [[nodiscard]] const vk::raii::DeviceMemory& getBaseMemory() const { return _memory; }

    private:
        static uint32_t findMemoryType(const RenderDevice& device, uint32_t typeFilter,
                                       vk::MemoryPropertyFlags properties) noexcept;

        friend class Image; /// TODO: Update Leaky Abstraction?

    private:
        size_t _size{};
        vk::raii::Buffer _buffer{ nullptr };
        vk::raii::DeviceMemory _memory{ nullptr };
        RenderDevice& _device;
    };
} // namespace tempest::renderer