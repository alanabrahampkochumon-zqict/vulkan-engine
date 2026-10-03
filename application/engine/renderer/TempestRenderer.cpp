/**
 * @file TempestRenderer.cpp
 * @author Alan Abraham P Kochumon
 * @date Created on: October 01, 2026
 *
 * @brief Implementation of member functions declared in TempestRenderer.h
 *
 * @copyright Copyright (c) 2026 Alan Abraham P Kochumon
 */

#include "TempestRenderer.h"

#include "../utils/Logger.h"

namespace tempest::renderer
{
    TempestRenderer::TempestRenderer(platform::TempestWindow& window, std::string appName, const uint32_t appVersion,
                                     std::string engineName, const uint32_t engineVersion) noexcept
        : _window{ window },
          _surface{ _window },
          _device{ _context, _surface },
          _appName{ std::move(appName) },
          _engineName{ std::move(engineName) },
          _appVersion{ appVersion },
          _engineVersion{ engineVersion }
    {}


    bool TempestRenderer::init(const RendererConfig& config) noexcept
    {

        if (!_context.init(_appName, _appVersion, _engineName, _engineVersion, VK_API_VERSION_1_3))
        {
            log::error("There was an error initializing the graphics context");
            return false;
        }

        // if (_surface.init(_context))
        //     return true;
    }

    void TempestRenderer::beginFrame() const noexcept {}

    void TempestRenderer::endFrame() const noexcept {}

    void TempestRenderer::applyConfigAndRecreateSwapChain(const RendererConfig& config) {}


    void TempestRenderer::draw() const noexcept { log::info("Drawing..."); }
} // namespace tempest::renderer
