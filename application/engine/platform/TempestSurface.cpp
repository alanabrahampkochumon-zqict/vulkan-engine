/**
 * @file TempestSurface.cpp
 * @author Alan Abraham P Kochumon
 * @date Created on: September 23, 2026
 *
 * @brief Implementation of surface abstraction declared in TempestSurface.h
 *
 * @copyright Copyright (c) 2026 Alan Abraham P Kochumon
 */

#include "TempestSurface.h"

#include "../utils/Logger.h"

#include <SDL3/SDL_vulkan.h>

namespace tempest::platform
{
    TempestSurface::TempestSurface(const TempestWindow& window, const vk::raii::Instance& instance) noexcept
        : _vulkanSurface(nullptr)
    {
        if (!createSurface(window, instance))
        {
            log::error("There was an error creating vulkan surface!");
            return;
        }
    }


    bool TempestSurface::createSurface(const TempestWindow& window, const vk::raii::Instance& instance) noexcept
    {
        VkSurfaceKHR surface;
        if (!SDL_Vulkan_CreateSurface(window.getCoreWindow(), *instance, nullptr, &surface))
        {
            return false;
        }
        _vulkanSurface = vk::raii::SurfaceKHR(instance, surface);
        return _vulkanSurface != nullptr;
    }


} // namespace tempest::platform