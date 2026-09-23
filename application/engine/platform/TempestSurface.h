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

#include <SDL3/SDL.h>
#include <vulkan/vulkan_raii.hpp>

namespace tempest::platform
{
    class TempestSurface
    {
    public:
        TempestSurface(SDL_Window& window, const vk::raii::Instance& instance) noexcept;

        const vk::raii::SurfaceKHR& getSurface() const noexcept { return _vulkanSurface; }

    protected:
        bool createSurface(SDL_Window& window, const vk::raii::Instance& instance) noexcept;

    private:
        vk::raii::SurfaceKHR _vulkanSurface;
    };
} // namespace tempest::platform