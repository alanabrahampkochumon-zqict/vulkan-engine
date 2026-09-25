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

#include <vulkan/vulkan_raii.hpp>
#include "Window.h"

namespace tempest::platform
{
    class TempestSurface
    {
    public:
        TempestSurface(const TempestWindow& window, const vk::raii::Instance& instance) noexcept;

        const vk::raii::SurfaceKHR& getSurface() const noexcept { return _vulkanSurface; }

    protected:
        bool createSurface(const TempestWindow& window, const vk::raii::Instance& instance) noexcept;

    private:
        vk::raii::SurfaceKHR _vulkanSurface;
    };
} // namespace tempest::platform