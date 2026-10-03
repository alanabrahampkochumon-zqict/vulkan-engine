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
    TempestRenderer::TempestRenderer(platform::TempestWindow& window, GraphicsContext& context) noexcept
        : _window{ window }, _surface{ _window }, _context{ context }, _device(), _pipeline()
    {}


    bool TempestRenderer::init() noexcept { return true; }


    void TempestRenderer::draw() const noexcept { log::info("Drawing..."); }
} // namespace tempest::renderer
