#pragma once
/**
 * @file RenderPipeline.h
 * @author Alan Abraham P Kochumon
 * @date Created on: September 23, 2026
 *
 * @brief Manages rendering pipeline.
 *
 * @copyright Copyright (c) 2026 Alan Abraham P Kochumon
 */

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include "Buffer.h"
#include "Image.h"
#include "PipelineConfig.h"
#include "RenderDevice.h"
#include "Swapchain.h"

#include <string>
#include <vulkan/vulkan_raii.hpp>

namespace tempest::renderer
{

    class RenderPipeline
    {
    public:
        RenderPipeline(const RenderDevice& device, const SwapChain& swapChain) noexcept;
        bool init(const PipelineConfig& config) noexcept;

        vk::Format findSupportedFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling,
                                       vk::FormatFeatureFlags features) const;
    protected:
        bool createGraphicsPipeline() noexcept;
        vk::raii::ShaderModule createShaderModule(const std::vector<char>& code) const noexcept;

    private:
        PipelineConfig _config;
        vk::raii::PipelineLayout _pipelineLayout{ nullptr };
        vk::raii::Pipeline _pipeline{ nullptr };
        const RenderDevice& _renderDevice;
        const SwapChain& _swapChain;

        // Separate?
        vk::raii::DescriptorSetLayout _descriptorSetLayout{ nullptr };
        vk::raii::DescriptorPool _descriptorPool{ nullptr };
    };
} // namespace tempest::renderer