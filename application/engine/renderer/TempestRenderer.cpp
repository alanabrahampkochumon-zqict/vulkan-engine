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
          _surface{ _window, _context },
          _device{ _context, _surface },
          _pipeline{ _device, _swapChain },
          _swapChain{ _device, _surface, _window },
          _vertexBuffer{ _device },
          _indexBuffer{ _device },
          _uniformBuffer{ _device },
          // _colorImage(),
          // _depthImage(),
          _appName{ std::move(appName) },
          _engineName{ std::move(engineName) },
          _appVersion{ appVersion },
          _engineVersion{ engineVersion }
    {}


    bool TempestRenderer::init(const RendererConfig& config) noexcept
    {

        if (!_context.init(_appName, _appVersion, _engineName, _engineVersion, MIN_GRAPHICS_API_VERSION))
        {
            log::error("There was an error initializing the graphics context");
            return false;
        }

        if (!_surface.init())
        {
            log::error("There was an error creating the surface.");
            return false;
        }

        constexpr QueueConfig queueConfig{ .enableGraphicsQueue         = true,
                                           .enableSeparateTransferQueue = false,
                                           .enableComputeQueue          = false };
        if (!_device.init(_context.getRequiredExtensions(), queueConfig, MIN_GRAPHICS_API_VERSION))
        {
            log::error("There was an error initializing the graphics devices.");
            return false;
        }


        const PipelineConfig pipelineConfig{
            .vertPath     = "shaders/slang.spv",
            .fragPath     = "shaders/slang.spv",
            .vertMainName = "vertMain",
            .fragMainName = "fragMain",
            .sampleCount  = vk::SampleCountFlagBits::e1 // TODO: Update
        };
        if (!_pipeline.init(pipelineConfig))
        {
            log::error("There was an error initializing the graphics pipeline.");
            return false;
        }

        if (!_swapChain.init(config.presentMode))
        {
            log::error("There was an error creating the swap chain.");
            return false;
        }

        return true;
    }


    vk::Format TempestRenderer::findDepthFormat() const noexcept
    {
        return _pipeline.findSupportedFormat(
            { vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint },
            vk::ImageTiling::eOptimal, vk::FormatFeatureFlagBits::eDepthStencilAttachment);
    }


    bool TempestRenderer::createDepthResources(const vk::SampleCountFlagBits msaaSamples) noexcept
    {
        const vk::Format format     = findDepthFormat();
        const auto& [width, height] = _swapChain.getExtent();
        _depthImage                 = std::move(Image(
            _device, width, height, format, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment,
            vk::MemoryPropertyFlagBits::eDeviceLocal, 1, msaaSamples, vk::ImageAspectFlagBits::eDepth));
        return true;
    }


    bool TempestRenderer::createColorResources(const vk::SampleCountFlagBits msaaSamples) noexcept
    {
        const vk::Format format     = _swapChain.getFormat().format;
        const auto& [width, height] = _swapChain.getExtent();
        _colorImage =
            std::move(Image(_device, width, height, format, vk::ImageTiling::eOptimal,
                            vk::ImageUsageFlagBits::eTransientAttachment | vk::ImageUsageFlagBits::eColorAttachment,
                            vk::MemoryPropertyFlagBits::eDeviceLocal, 1, msaaSamples, vk::ImageAspectFlagBits::eColor));
        return true;
    }


    void TempestRenderer::beginFrame() const noexcept {}

    void TempestRenderer::endFrame() const noexcept {}

    void TempestRenderer::applyConfigAndRecreateSwapChain(const RendererConfig& config) {}


    void TempestRenderer::draw() const noexcept { log::info("Drawing..."); }
} // namespace tempest::renderer
