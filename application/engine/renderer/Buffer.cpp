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

#include "../utils/Logger.h"
#include "CommandBuffer.h"


namespace tempest::renderer
{
    Buffer::Buffer(RenderDevice& device, const size_t size, const vk::BufferUsageFlagBits usageFlags,
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
        const vk::MemoryAllocateInfo allocateInfo{ .allocationSize  = memRequirements.size,
                                                   .memoryTypeIndex = findMemoryType(
                                                       _device, memRequirements.memoryTypeBits, memProperties) };
        _memory = vk::raii::DeviceMemory(_device.getDevice(), allocateInfo);
        _buffer.bindMemory(*_memory, 0);
    }


    uint32_t Buffer::findMemoryType(const RenderDevice& device, const uint32_t typeFilter,
                                    const vk::MemoryPropertyFlags properties) noexcept
    {
        // Graphics cards provide different memory types, so we need to query and choose one
        // that best fits our requirements
        const vk::PhysicalDeviceMemoryProperties memProperties = device.getPhysicalDevice().getMemoryProperties();
        // Has memoryTypes and memoryHeaps

        // Return the index of memory type if it matches the properties we need.
        for (uint32_t i = 0; i < memProperties.memoryTypeCount; ++i)
        {
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
            {
                return i;
            }
        }
        log::error("Failed to find a suitable memory type");
        // TODO: Check whether this is a valid type and only return an invalid type.
        return std::numeric_limits<uint32_t>::max();
    }


    void Buffer::copyTo(const Buffer& destination, const size_t size, RenderQueue& queue) const noexcept
    {
        const CommandBuffer copyCommandBuffer{ _device, queue, 1 };
        copyCommandBuffer.beginSingleTimeRecording();
        copyCommandBuffer.getBaseCommandBuffers()[0].copyBuffer(_buffer, destination._buffer,
                                                                vk::BufferCopy(0, 0, size));
        copyCommandBuffer.endSingleTimeRecordingAndSubmit();
    }


    void Buffer::copy(const Buffer& source, const Buffer& destination, const size_t size,
                      RenderQueue& queue) noexcept
    { source.copyTo(destination, size, queue); }


    /// TODO: Extend with offset and mappingsize?
    void Buffer::memcpy(const void* source, const size_t size) const noexcept
    {
        void* memory = _memory.mapMemory(0, size);
        std::memcpy(memory, source, size);
        _memory.unmapMemory();
    }
} // namespace tempest::renderer