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
#include "Vertex.h"
#include "config/RendererConfig.h"

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
        TempestRenderer(platform::TempestWindow& window, std::string appName, uint32_t appVersion,
                        std::string engineName, uint32_t engineVersion) noexcept;
        bool init(const RendererConfig& config) noexcept;

        void beginFrame() const noexcept;
        void endFrame() const noexcept;
        void draw() const noexcept;
        void applyConfigAndRecreateSwapChain(const RendererConfig& config);


    private:
        vk::Format findDepthFormat() const noexcept;

        bool createDepthResources(vk::SampleCountFlagBits msaaSamples) noexcept;
        bool createColorResources(vk::SampleCountFlagBits msaaSamples) noexcept;
        // bool createDescriptor() noexcept;
        bool createIndexBuffer(const std::vector<uint32_t>& indices) noexcept;
        bool createVertexBuffer(const std::vector<Vertex>& vertices) noexcept;
        bool createUniformBuffer() noexcept;

    private:
        platform::TempestWindow& _window;

        TempestSurface _surface;
        GraphicsContext _context;
        RenderDevice _device;
        RenderPipeline _pipeline;
        SwapChain _swapChain;
        Buffer _vertexBuffer, _indexBuffer;
        std::vector<Buffer> _uniformBuffers;
        Image _colorImage, _depthImage;

        std::string _appName, _engineName;
        uint32_t _appVersion, _engineVersion;


        static constexpr auto MIN_GRAPHICS_API_VERSION = VK_API_VERSION_1_3;
        static constexpr auto MAX_FRAMES_IN_FLIGHT     = 2;
    };
} // namespace tempest::renderer