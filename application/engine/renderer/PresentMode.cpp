/**
 * @file PresentMode.cpp
 * @author Alan Abraham P Kochumon
 * @date Created on: September 26, 2026
 *
 * @brief Implementation of member functions declared in PresentMode.h
 *
 * @copyright Copyright (c) 2026 Alan Abraham P Kochumon
 */

#include "PresentMode.h"

#include <vulkan/vulkan.hpp>

namespace tempest::renderer
{
    constexpr PresentMode fromVKPresentMode(const vk::PresentModeKHR vkPresentMode) noexcept
    {
        switch (vkPresentMode)
        {
            case vk::PresentModeKHR::eFifo:
                return PresentMode::VSYNC;
            case vk::PresentModeKHR::eMailbox:
                return PresentMode::MAILBOX;
            case vk::PresentModeKHR::eImmediate:
                return PresentMode::IMMEDIATE;
            default:
                return PresentMode::VSYNC;
        }
    }


    constexpr vk::PresentModeKHR toVKPresentMode(const PresentMode presentMode) noexcept
    {
        switch (presentMode)
        {
            case PresentMode::VSYNC:
                return vk::PresentModeKHR::eFifo;
            case PresentMode::MAILBOX:
                return vk::PresentModeKHR::eMailbox;
            case PresentMode::IMMEDIATE:
                return vk::PresentModeKHR::eImmediate;
            default:
                return vk::PresentModeKHR::eFifo;
        }
    }
} // namespace tempest::renderer