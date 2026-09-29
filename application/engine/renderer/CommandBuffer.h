#pragma once
/**
 * @file CommandBuffer.h
 * @author Alan Abraham P Kochumon
 * @date Created on: September 29, 2026
 *
 * @brief Command Buffer abstraction.
 *
 * @copyright Copyright (c) 2026 Alan Abraham P Kochumon
 */

#include "RenderDevice.h"

#include <vulkan/vulkan_raii.hpp>

namespace tempest::renderer
{

    class CommandBuffer
    {
    public:
        CommandBuffer(const RenderDevice& device, const RenderQueue& queue, size_t count) noexcept;
        void startRecording(uint32_t index) const noexcept;
        void endRecording(uint32_t index) const noexcept;

        /// Begin Single Time Command Buffer Recording.
        void beginSingleTimeRecording(uint32_t index = 0) const noexcept;
        /// Ends Single Time Command Buffer Recording.
        void endSingleTimeRecordingAndSubmit(uint32_t index = 0) const noexcept;

        [[nodiscard]] const std::vector<vk::raii::CommandBuffer>& getBaseCommandBuffers() const noexcept
        { return _commandBuffers; }
        [[nodiscard]] size_t getCommandBufferCount() const noexcept { return _count; }


    private:
        void createCommandPool() noexcept;
        void createCommandBuffers() noexcept;

    private:
        vk::raii::CommandPool _commandPool{ nullptr };
        std::vector<vk::raii::CommandBuffer> _commandBuffers{};
        const size_t _count;
        const RenderDevice& _device;
        // While the device has the render queue information, since we are allocating from a commandpool for that
        // queue, user has to explicitly provide it.
        const RenderQueue& _queue;
    };
} // namespace tempest::renderer