/**
 * @file CommandBuffer.cpp
 * @author Alan Abraham P Kochumon
 * @date Created on: September 29, 2026
 *
 * @brief Implementation of member functions declared in CommandBuffer.h
 *
 * @copyright Copyright (c) 2026 Alan Abraham P Kochumon
 */

#include "CommandBuffer.h"


namespace tempest::renderer
{

    CommandBuffer::CommandBuffer(const RenderDevice& device, const RenderQueue& queue, const size_t count) noexcept
        : _count{ count }, _device{ device }, _queue{ queue }
    {
        createCommandPool();
        createCommandBuffers();
    }

    void CommandBuffer::startRecording(const uint32_t index) const noexcept { _commandBuffers[index].begin({}); }

    void CommandBuffer::endRecording(const uint32_t index) const noexcept { _commandBuffers[index].end(); }

    void CommandBuffer::beginSingleTimeRecording(const uint32_t index) const noexcept
    {
        constexpr vk::CommandBufferBeginInfo beginInfo{ .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit };
        _commandBuffers[index].begin(beginInfo);
    }

    void CommandBuffer::endSingleTimeRecordingAndSubmit(const uint32_t index) const noexcept
    {
        // End the command buffer
        _commandBuffers[index].end();

        // Submit the command
        const vk::SubmitInfo submitInfo{ .commandBufferCount = 1, .pCommandBuffers = &*_commandBuffers[index] };
        _queue.queue.submit(submitInfo);
        _queue.queue.waitIdle();
    }


    void CommandBuffer::createCommandPool() noexcept
    {
        // For allocation command buffer we need a command pool first
        const vk::CommandPoolCreateInfo commandPoolCreateInfo{
            // Allow individual re-recording
            // Transient-> Allows command buffers to be rerecorded with new commands often.
            .flags            = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
            .queueFamilyIndex = _queue.familyIndex
        };

        // Command buffers execute by submitting them to ONE of the device queues, like graphics or presentation,
        // and each queue type require a different command buffer
        _commandPool = vk::raii::CommandPool(_device.getDevice(), commandPoolCreateInfo);
    }

    void CommandBuffer::createCommandBuffers() noexcept
    {
        const vk::CommandBufferAllocateInfo commandBufferInfo{
            .commandPool = _commandPool,
            // Can be submitted to a queue for execution, but cannot be called from other Command Buffers
            // Secondary-> Cannot be submitted, but can be called from Primary Command Buffers
            .level = vk::CommandBufferLevel::ePrimary,
            // TODO: Add abstraction for primary vs second command buffers
            .commandBufferCount = static_cast<uint32_t>(_count)
        };

        _commandBuffers = std::move(vk::raii::CommandBuffers(_device.getDevice(), commandBufferInfo));
    }
} // namespace tempest::renderer
