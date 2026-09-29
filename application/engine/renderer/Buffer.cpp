/**
 * @file Buffer.cpp
 * @author Alan Abraham P Kochumon
 * @date Created on: September 29, 2026
 *
 * @brief Implementation of member functions declared in Buffer.h
 *
 * @copyright Copyright (c) 2026 Alan Abraham P Kochumon
 */

#include "Buffer.h"


namespace tempest::renderer
{
    Buffer::Buffer(const RenderDevice& device, const size_t size, const vk::BufferUsageFlagBits usageFlags,
                   const vk::MemoryPropertyFlags memProperties) noexcept
        : _size{ size }, _device{ device }
    {
        // In vulkan we need to create a buffer, then allocate memory as per it's requirements
        // and bind the memory to the buffer.
        const vk::BufferCreateInfo bufferInfo{ .size        = size,
                                               .usage       = usageFlags,
                                               .sharingMode = vk::SharingMode::eExclusive };
        _buffer                                      = vk::raii::Buffer(_device.getDevice(), bufferInfo);
        const vk::MemoryRequirements memRequirements = _buffer.getMemoryRequirements();
        const vk::MemoryAllocateInfo allocateInfo{ .allocationSize = memRequirements.size,
                                                   .memoryTypeIndex =
                                                       findMemoryType(memRequirements.memoryTypeBits, memProperties) };
        _memory = vk::raii::DeviceMemory(_device.getDevice(), allocateInfo);
        _buffer.bindMemory(*_memory, 0);
    }


    uint32_t Buffer::findMemoryType(const uint32_t typeFilter, const vk::MemoryPropertyFlags properties) const
    {
        // Graphics cards provide different memory types, so we need to query and choose one
        // that best fits our requirements
        const vk::PhysicalDeviceMemoryProperties memProperties = _device.getPhysicalDevice().getMemoryProperties();
        // Has memoryTypes and memoryHeaps

        // Return the index of memory type if it matches the properties we need.
        for (uint32_t i = 0; i < memProperties.memoryTypeCount; ++i)
        {
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
            {
                return i;
            }
        }

        throw std::runtime_error("Failed to find a suitable memory type.");
    }


    void Buffer::copyTo(const Buffer& destination, size_t size) const noexcept
    {
        // TODO: Write after adding CommandBuffer
    }


    void Buffer::copy(const Buffer& source, const Buffer& destination, size_t size) noexcept { source.copyTo(destination, size); }
} // namespace tempest::renderer