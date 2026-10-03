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

namespace tempest::renderer
{
    TempestSurface::TempestSurface(platform::TempestWindow& window, const GraphicsContext& context) noexcept
        : _context{ context }, _window{ window }, _vulkanSurface{ nullptr }
    {}


    bool TempestSurface::init() noexcept
    {
        const auto& instance = _context.getInstance();

        VkSurfaceKHR surface;
        if (!SDL_Vulkan_CreateSurface(_window.getCoreWindow(), *instance, nullptr, &surface))
        {
            return false;
        }
        _vulkanSurface = vk::raii::SurfaceKHR(instance, surface);
        return _vulkanSurface != nullptr;
    }


} // namespace tempest::renderer