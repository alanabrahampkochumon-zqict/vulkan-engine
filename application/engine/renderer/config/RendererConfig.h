#pragma once
/**
 * @file RendererConfig.h
 * @author Alan Abraham P Kochumon
 * @date Created on: October 03, 2026
 *
 * @brief Configuration for Tempest Renderer.
 *
 * @copyright Copyright (c) 2026 Alan Abraham P Kochumon
 */


#include "../PresentMode.h"

#include <cstdint>

namespace tempest
{

    struct RendererConfig
    {
        uint32_t width;
        uint32_t height;
        renderer::PresentMode presentMode = renderer::PresentMode::VSYNC;
        bool enableValidationLayers       = true;
    };
} // namespace tempest