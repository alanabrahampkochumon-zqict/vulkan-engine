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
#include "UniformBufferObject.h"
#include "Vertex.h"

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


    // TODO: Update buffers to a general pool
    bool TempestRenderer::createIndexBuffer(const std::vector<uint32_t>& indices) noexcept
    {
        // Since the memory used by gpu for fast transfer are not host accessible we need
        // to create a staging buffer(HOST_VISIBLE) and then transfer the data into the fast DeviceLocal Memory.
        // const vk::DeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

        //------------- STAGING BUFFER ------------------
        // The driver may not copy the memory immediately due to caching.
        // SOL 1: use vk::MemoryPropertyFlagBits::eHostCoherent(Used here)
        // SOL 2: use vk::raii::Device::flushMappedMemoryRanges after writing to mapped memory
        //        vk::raii::Device::invalidateMappedMemoryRanges before reading from mapped memory.
        const auto size = indices.size() * sizeof(indices[0]);
        Buffer stagingBuffer(_device);
        if (!stagingBuffer.create(size, vk::BufferUsageFlagBits::eTransferSrc,
                                  vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent))
        {
            log::error("There was an error create the staging buffer for indices.");
            return false;
        }
        stagingBuffer.write(indices.data(), indices.size());


        //------------- GPU BUFFER ------------------
        if (!_indexBuffer.create(size, vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst,
                                 vk::MemoryPropertyFlagBits::eDeviceLocal))
        {
            log::error("There was an error creating the index buffer");
            return false;
        }
        stagingBuffer.copyTo(_indexBuffer, size, _device.getQueues()[0]);
        return true;
    }


    bool TempestRenderer::createVertexBuffer(const std::vector<Vertex>& vertices) noexcept
    {
        const auto size = vertices.size() * sizeof(vertices[0]);
        Buffer stagingBuffer(_device);
        if (!stagingBuffer.create(size, vk::BufferUsageFlagBits::eTransferSrc,
                                  vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent))
        {
            log::error("There was an error create the staging buffer for vertex.");
            return false;
        }
        stagingBuffer.write(vertices.data(), vertices.size());


        //------------- GPU BUFFER ------------------
        if (!_vertexBuffer.create(size, vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst,
                                  vk::MemoryPropertyFlagBits::eDeviceLocal))
        {
            log::error("There was an error creating the vertex buffer");
            return false;
        }
        stagingBuffer.copyTo(_vertexBuffer, size, _device.getQueues()[0]);
        return true;
    }

    bool TempestRenderer::createUniformBuffer() noexcept
    {
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
        {
            constexpr vk::DeviceSize size = sizeof(UniformBufferObject);
            _uniformBuffers.emplace_back(std::move(Buffer(_device)));

            auto [buffer, bufferMemory] =
                createBuffer(size, vk::BufferUsageFlagBits::eUniformBuffer,
                             vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
            uniformBuffers.emplace_back(std::move(buffer));
            uniformBuffersMemory.emplace_back(std::move(bufferMemory));
            // Persistent mapping since we are updating buffer every frame, it is better to persistent mapping.
            uniformBuffersMapped.emplace_back(uniformBuffersMemory.back().mapMemory(0, size));
        }
    }


    void TempestRenderer::beginFrame() const noexcept {}

    void TempestRenderer::endFrame() const noexcept {}

    void TempestRenderer::applyConfigAndRecreateSwapChain(const RendererConfig& config) {}


    void TempestRenderer::draw() const noexcept { log::info("Drawing..."); }
} // namespace tempest::renderer
