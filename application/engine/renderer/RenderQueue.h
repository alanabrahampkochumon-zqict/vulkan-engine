#pragma once
/**
 * @file RenderQueue.h
 * @author Alan Abraham P Kochumon
 * @date Created on: September 24, 2026
 *
 * @brief A datastructure for associating a render queue with its index.
 *
 * @copyright Copyright (c) 2026 Alan Abraham P Kochumon
 */

#include <cstdint>
#include <vulkan/vulkan_raii.hpp>

namespace tempest::renderer
{

    enum class RenderQueueType
    {
        GRAPHICS,
        TRANSFER,
        COMPUTE
    };

    static constexpr uint32_t INVALID_QUEUE_INDEX = ~0UL;

    /// Structure representing a render queue.
    struct RenderQueue
    {
        vk::raii::Queue queue{ nullptr };
        RenderQueueType type{ RenderQueueType::GRAPHICS };
        uint32_t familyIndex{ INVALID_QUEUE_INDEX };
    };


    /// Configuration for RenderQueue
    struct QueueConfig
    {
        bool enableGraphicsQueue{ true };
        bool enableSeparateTransferQueue{ false };
        bool enableComputeQueue{ false };
        // Add other queues are necessary
    };
} // namespace tempest::renderer