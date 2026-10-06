/**
 * @file RenderPipeline.cpp
 * @author Alan Abraham P Kochumon
 * @date Created on: September 23, 2026
 *
 * @brief Implementation of functions declared in RenderPipeline.h
 *
 * @copyright Copyright (c) 2026 Alan Abraham P Kochumon
 */

#include "RenderPipeline.h"

#include "../utils/FileReader.h"
#include "../utils/Logger.h"
#include "Vertex.h"

#include <iostream>

namespace tempest::renderer
{
    using namespace tempest::utils;

    RenderPipeline::RenderPipeline(const RenderDevice& device, const SwapChain& swapChain) noexcept
        : _config(), _renderDevice{ device }, _swapChain{ swapChain }
    {}


    bool RenderPipeline::init(const PipelineConfig& config) noexcept
    {
        _config = config;
        return createGraphicsPipeline();
    }


    bool RenderPipeline::createGraphicsPipeline() noexcept
    {
        const auto vertFile = readFile(_config.vertPath);
        const auto fragFile = readFile(_config.fragPath);

        const auto vertShader = createShaderModule(vertFile);
        const auto fragShader = createShaderModule(fragFile);

        /// Note: pSpecializationInfo can be used to specify shader constants.
        const vk::PipelineShaderStageCreateInfo vertexShaderCreateInfo{ .stage  = vk::ShaderStageFlagBits::eVertex,
                                                                        .module = vertShader,
                                                                        .pName  = _config.vertMainName.c_str() };
        const vk::PipelineShaderStageCreateInfo fragmentShaderCreateInfo{ .stage  = vk::ShaderStageFlagBits::eFragment,
                                                                          .module = fragShader,
                                                                          .pName  = _config.fragMainName.c_str() };

        vk::PipelineShaderStageCreateInfo shaderStages[] = { vertexShaderCreateInfo, fragmentShaderCreateInfo };
        vk::PipelineDepthStencilStateCreateInfo depthStencil{
            .depthTestEnable       = vk::True,
            .depthWriteEnable      = vk::True,
            .depthCompareOp        = vk::CompareOp::eLess,
            .depthBoundsTestEnable = vk::False, //
            .stencilTestEnable     = vk::False,
        };

        /// States like viewport dimensions, line width, and blend constants can be changed
        /// without recreating the graphics pipeline at draw time, but we need to specify a dynamic state to do so.
        /// By creating a dynamic state, teh configuration of these values will be ignored, requiring those to be
        /// specified at draw time.
        std::vector dynamicStates = { vk::DynamicState::eViewport, vk::DynamicState::eScissor };
        vk::PipelineDynamicStateCreateInfo dynamicState{ .dynamicStateCount =
                                                             static_cast<uint32_t>(dynamicStates.size()),
                                                         .pDynamicStates = dynamicStates.data() };

        // Describes teh format of vertex data passed into vertex shader
        // Binding: Describe the spacing between data and whether they are per vertex or per instance
        // Attribute Description: Type of attributes passed to the vertex, with the binding and offset
        auto bindingDescription   = Vertex::getBindingDescription();
        auto attributeDescription = Vertex::getAttributeDescription();
        vk::PipelineVertexInputStateCreateInfo vertexInputInfo{
            .vertexBindingDescriptionCount   = static_cast<uint32_t>(1),
            .pVertexBindingDescriptions      = &bindingDescription,
            .vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescription.size()),
            .pVertexAttributeDescriptions    = attributeDescription.data()
        };

        // Topology or primitive types(TriangleList, Fan, Line, Point etc.)
        // primitiveRestartEnable: Breaks up lines and tris using special index of 0xffff, or 0xffffffff
        vk::PipelineInputAssemblyStateCreateInfo inputAssembly{ .topology = vk::PrimitiveTopology::eTriangleList };

        // Create a viewport with the swap chain dimensions
        // Viewport describe the transformation from the image(swap chain) to framebuffer
        vk::Viewport viewport{ .x        = 0.0f,
                               .y        = 0.0f,
                               .width    = static_cast<float>(_swapChain.getExtent().width),
                               .height   = static_cast<float>(_swapChain.getExtent().height),
                               .minDepth = 0.0f,
                               .maxDepth = 1.0f };

        // scissor defined the region of pixels to store(filtering)
        vk::Rect2D scissor{ .offset = vk::Offset2D{ .x = 0, .y = 0 }, .extent = _swapChain.getExtent() };

        vk::PipelineViewportStateCreateInfo viewportState{
            .viewportCount = 1, .pViewports = &viewport, .scissorCount = 1, .pScissors = &scissor
        };


        // Rasterizer
        vk::PipelineRasterizationStateCreateInfo rasterizer{
            // If set to true, then fragments beyond far and near planes are clamped, and not discarded
            // useful for shadow maps (requires GPU feature)
            .depthClampEnable = vk::False,
            // If set to true, not geometry pass through the rasterizer, disables all output to framebuffer
            .rasterizerDiscardEnable = vk::False,
            .polygonMode             = vk::PolygonMode::eFill,      // Fill vs Wireframe vs Dots
            .cullMode                = vk::CullModeFlagBits::eBack, // Back face culling,
            // Winding direction (since we are using -y) to draw the triangles in the opposite direction
            .frontFace       = vk::FrontFace::eCounterClockwise,
            .depthBiasEnable = vk::False, // Bias the depth value based on slope(useful for shadow maps)
            .lineWidth       = 1.0f,      // Lines thicker than 1.0f require wideLines GPU feature
        };

        // Multisampling
        // Enable sample shading
        vk::PipelineMultisampleStateCreateInfo multisampling{ .rasterizationSamples = _config.sampleCount,
                                                              .sampleShadingEnable  = vk::True,
                                                              .minSampleShading     = 0.2f };

        // Depth and stencil tests
        // Unused right now
        // vk::PipelineDepthStencilStateCreateInfo depthStencilTests{};

        // Color blending
        // Color blending per attached framebuffer
        // Alpha blending(alpha * c1 + (1 - alpha) * c1)
        vk::PipelineColorBlendAttachmentState colorAttachmentState{
            .blendEnable         = vk::False,
            .srcColorBlendFactor = vk::BlendFactor::eSrcAlpha,
            .dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha,
            .colorBlendOp        = vk::BlendOp::eAdd,
            .srcAlphaBlendFactor = vk::BlendFactor::eOne,
            .dstAlphaBlendFactor = vk::BlendFactor::eZero,
            .alphaBlendOp        = vk::BlendOp::eAdd,
            // Determines which components will be affected
            .colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
                vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
        };

        // Global color blending vk::PipelineColorBlendStateCreateInfo
        vk::PipelineColorBlendStateCreateInfo colorBlending{
            .logicOpEnable   = vk::False, // Enabling this blending will turn off first blending
            .logicOp         = vk::LogicOp::eCopy,
            .attachmentCount = 1,
            .pAttachments    = &colorAttachmentState
        };

        // Pipeline layout
        vk::PipelineLayoutCreateInfo pipelineLayoutInfo{ .setLayoutCount         = 1,
                                                         .pSetLayouts            = &*_descriptorSetLayout,
                                                         .pushConstantRangeCount = 0 };
        _pipelineLayout = vk::raii::PipelineLayout(_renderDevice.getDevice(), pipelineLayoutInfo);

        // Dynamic rendering
        // Create the pipeline with graphics and rendering pipelines
        vk::StructureChain<vk::GraphicsPipelineCreateInfo, vk::PipelineRenderingCreateInfo> pipelineCreateInfoChain = {
            { .stageCount          = 2,
              .pStages             = shaderStages,
              .pVertexInputState   = &vertexInputInfo,
              .pInputAssemblyState = &inputAssembly,
              .pViewportState      = &viewportState,
              .pRasterizationState = &rasterizer,
              .pMultisampleState   = &multisampling,
              .pDepthStencilState  = &depthStencil,
              .pColorBlendState    = &colorBlending,
              .pDynamicState       = &dynamicState,
              .layout              = _pipelineLayout,
              .renderPass          = nullptr },

            { .colorAttachmentCount    = 1,
              .pColorAttachmentFormats = &_swapChain.getFormat().format,
              .depthAttachmentFormat   = findDepthFormat() }
        };
        // BasePipelineHandle and BasePipelineIndex -> used for inheriting pipelines

        _pipeline = vk::raii::Pipeline(_renderDevice.getDevice(), nullptr,
                                       pipelineCreateInfoChain.get<vk::GraphicsPipelineCreateInfo>());

        if (_pipeline == nullptr)
        {
            log::error("There was an error creating graphics pipeline");
            return false;
        }
        return true;
    }


    vk::raii::ShaderModule RenderPipeline::createShaderModule(const std::vector<char>& code) const noexcept
    {
        const vk::ShaderModuleCreateInfo shaderCreateInfo{
            .codeSize = code.size(),
            .pCode    = reinterpret_cast<uint32_t const*>(code.data()),
        };
        vk::raii::ShaderModule module{ _renderDevice.getDevice(), shaderCreateInfo };
        return module;
    }


    vk::Format RenderPipeline::findDepthFormat() const noexcept
    {
        return findSupportedFormat(
            { vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint },
            vk::ImageTiling::eOptimal, vk::FormatFeatureFlagBits::eDepthStencilAttachment);
    }


    vk::Format RenderPipeline::findSupportedFormat(const std::vector<vk::Format>& candidates,
                                                   const vk::ImageTiling tiling,
                                                   const vk::FormatFeatureFlags features) const
    {
        for (const auto format : candidates)
        {
            const vk::FormatProperties properties = _renderDevice.getPhysicalDevice().getFormatProperties(format);
            // Support of format depends on the tiling mode and usage
            if (((tiling == vk::ImageTiling::eLinear) && ((properties.linearTilingFeatures & features) == features)) ||
                ((tiling == vk::ImageTiling::eOptimal) && ((properties.optimalTilingFeatures & features) == features)))
            {
                return format;
            }
        }
        throw std::runtime_error("Failed to find the supported format!");
    }


} // namespace tempest::renderer
