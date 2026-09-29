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
        Buffer(const RenderDevice& device, size_t size, vk::BufferUsageFlagBits usageFlags,
               vk::MemoryPropertyFlags memProperties) noexcept;

        Buffer(const Buffer& other)            = delete;
        Buffer& operator=(const Buffer& other) = delete;

        Buffer(Buffer&& other) noexcept
            : _size(other._size), _buffer(std::move(other._buffer)), _memory(std::move(other._memory))
        {}

        Buffer& operator=(Buffer&& other) noexcept
        {
            if (this == &other)
                return *this;
            _size   = other._size;
            _buffer = std::move(other._buffer);
            _memory = std::move(other._memory);
            return *this;
        }


        /// Copy the current buffer to a new destinations.
        void copyTo(const Buffer& destination, size_t size) const noexcept;

        /// Copy @p source buffer to @p destination.
        static void copy(const Buffer& source, const Buffer& destination, size_t size) noexcept;

        [[nodiscard]] const vk::raii::Buffer& getBaseBuffer() const { return _buffer; }

    private:
        uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) const;

    private:
        size_t _size{};
        vk::raii::Buffer _buffer{ nullptr };
        vk::raii::DeviceMemory _memory{ nullptr };
        const RenderDevice& _device;
    };
} // namespace tempest::renderer