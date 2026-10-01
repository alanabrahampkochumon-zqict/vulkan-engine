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
        RenderPipeline(RenderDevice& device, SwapChain& swapChain, PipelineConfig& config) noexcept;


    protected:
        bool createGraphicsPipeline() noexcept;

        vk::raii::ShaderModule createShaderModule(const std::vector<char>& code) const noexcept;
        vk::Format findDepthFormat() const noexcept;
        vk::Format findSupportedFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling,
                                                      vk::FormatFeatureFlags features) const;

    private:
        PipelineConfig& _config;
        vk::raii::PipelineLayout _pipelineLayout{ nullptr };
        vk::raii::Pipeline _graphicsPipeline{ nullptr };
        RenderDevice& _renderDevice;
        SwapChain& _swapChain;

        // Separate?
        vk::raii::DescriptorSetLayout _descriptorSetLayout{ nullptr };
        vk::raii::DescriptorPool _descriptorPool{ nullptr };
    };
} // namespace tempest::renderer