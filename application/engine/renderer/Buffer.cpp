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
    Buffer::Buffer(const RenderDevice& device) noexcept: _device{ device } {}

    bool Buffer::create(const size_t size, const vk::BufferUsageFlags usageFlags,
                        const vk::MemoryPropertyFlags memProperties) noexcept
    {
        // In vulkan we need to create a buffer, then allocate memory as per it's requirements
        // and bind the memory to the buffer.
        const vk::BufferCreateInfo bufferInfo{ .size        = size,
                                               .usage       = usageFlags,
                                               .sharingMode = vk::SharingMode::eExclusive };
        _buffer                                      = vk::raii::Buffer(_device.getBaseDevice(), bufferInfo);
        const vk::MemoryRequirements memRequirements = _buffer.getMemoryRequirements();
        const vk::MemoryAllocateInfo allocateInfo{ .allocationSize  = memRequirements.size,
                                                   .memoryTypeIndex = _device.findMemoryType(
                                                       memRequirements.memoryTypeBits, memProperties) };
        _memory = vk::raii::DeviceMemory(_device.getBaseDevice(), allocateInfo);
        _buffer.bindMemory(*_memory, 0);

        return _buffer != nullptr && _memory != nullptr;
    }


    void Buffer::write(const void* data, const size_t size) const
    {
        void* dst = _memory.mapMemory(0, _size);
        std::memcpy(dst, data, size);
        _memory.unmapMemory();
    }


    void Buffer::copyTo(const Buffer& destination, const size_t size, RenderQueue& queue) const noexcept
    {
        const CommandBuffer copyCommandBuffer{ _device, queue, 1 };
        copyCommandBuffer.beginSingleTimeRecording();
        copyCommandBuffer.getBaseCommandBuffers()[0].copyBuffer(_buffer, destination._buffer,
                                                                vk::BufferCopy(0, 0, size));
        copyCommandBuffer.endSingleTimeRecordingAndSubmit();
    }


    void Buffer::copy(const Buffer& source, const Buffer& destination, const size_t size, RenderQueue& queue) noexcept
    { source.copyTo(destination, size, queue); }


    /// TODO: Extend with offset and mappingsize?
    void Buffer::memcpy(const void* source, const size_t size) const noexcept
    {
        void* memory = _memory.mapMemory(0, size);
        std::memcpy(memory, source, size);
        _memory.unmapMemory();
    }
} // namespace tempest::renderer