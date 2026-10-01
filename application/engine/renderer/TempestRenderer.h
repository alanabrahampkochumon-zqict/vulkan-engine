#pragma once
/**
 * @file TempestRenderer.h
 * @author Alan Abraham P Kochumon
 * @date Created on: October 01, 2026
 *
 * @brief The rendering framework for Tempest.
 *
 * @copyright Copyright (c) 2026 Alan Abraham P Kochumon
 */

#include "GraphicsContext.h"
#include "RenderDevice.h"
#include "RenderPipeline.h"
#include "RenderQueue.h"

namespace tempest::platform
{
    /// Forward Reference
    class TempestWindow;
    class TempestSurface;
} // namespace tempest::platform

namespace tempest::renderer
{
    class TempestRenderer
    {
    public:
        TempestRenderer(platform::TempestWindow& window, platform::TempestSurface& surface, GraphicsContext& context) noexcept;
        bool init() noexcept;

        void draw() const noexcept;

    private:
        platform::TempestWindow& _window;
        platform::TempestSurface& _surface;
        GraphicsContext& _context;
        // RenderDevice _device;
        // GraphicsContext _context;
    };
} // namespace tempest::renderer