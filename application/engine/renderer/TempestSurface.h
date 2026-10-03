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

#include "../platform/TempestWindow.h"
#include "GraphicsContext.h"

#include <vulkan/vulkan_raii.hpp>

namespace tempest::renderer
{
    class TempestSurface
    {
    public:
        explicit TempestSurface(platform::TempestWindow& window, const GraphicsContext& instance) noexcept;
        bool init() noexcept;

        const vk::raii::SurfaceKHR& getBaseSurface() const noexcept { return _vulkanSurface; }

    private:
        const GraphicsContext& _context;
        const platform::TempestWindow& _window;
        vk::raii::SurfaceKHR _vulkanSurface;
    };
} // namespace tempest::renderer