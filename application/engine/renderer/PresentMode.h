#pragma once
/**
 * @file PresentMode.h
 * @author Alan Abraham P Kochumon
 * @date Created on: September 26, 2026
 *
 * @brief Abstraction for Swapchain extensions.
 *
 * @copyright Copyright (c) 2026 Alan Abraham P Kochumon
 */

#include <cstdint>
#include <limits>

namespace vk
{
    enum class PresentModeKHR;
}

namespace tempest::renderer
{
    enum class PresentMode : uint8_t
    {
        VSYNC, /// FIFO
        MAILBOX,
        IMMEDIATE
    };

    constexpr PresentMode fromVKPresentMode(vk::PresentModeKHR vkPresentMode) noexcept;
    constexpr vk::PresentModeKHR toVKPresentMode(PresentMode presentMode) noexcept;

} // namespace tempest::renderer