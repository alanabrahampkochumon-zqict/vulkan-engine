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

#include <SDL3/SDL_vulkan.h>
#include <iostream>

namespace tempest::platform
{
    TempestSurface::TempestSurface(SDL_Window& window, const vk::raii::Instance& instance) noexcept
        : _vulkanSurface(nullptr)
    {
        if (!createSurface(window, instance))
        {
            std::cout << "There was an error creating a vulkan surface!\n";
            return;
        }
    }


    bool TempestSurface::createSurface(SDL_Window& window, const vk::raii::Instance& instance) noexcept
    {
        VkSurfaceKHR surface;
        if (!SDL_Vulkan_CreateSurface(&window, *instance, nullptr, &surface))
        {
            return false;
        }
        _vulkanSurface = vk::raii::SurfaceKHR(instance, surface);
        return true;
    }


} // namespace tempest::platform