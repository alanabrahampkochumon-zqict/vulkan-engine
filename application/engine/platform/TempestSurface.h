#pragma once
/**
 * @file TempestSurface.h
 * @author Alan Abraham P Kochumon
 * @date Created on: September 23, 2026
 *
 * @brief Rendering surface abstraction.
 *
 * @copyright Copyright (c) 2026 Alan Abraham P Kochumon
 */

#include "TempestWindow.h"

#include <vulkan/vulkan_raii.hpp>

namespace tempest::platform
{
    class TempestSurface
    {
    public:
        TempestSurface(TempestWindow& window) noexcept;
        bool init(const vk::raii::Instance& instance) noexcept;

        const vk::raii::SurfaceKHR& getBaseSurface() const noexcept { return _vulkanSurface; }

    private:
        TempestWindow& _window;
        vk::raii::SurfaceKHR _vulkanSurface;
    };
} // namespace tempest::platform