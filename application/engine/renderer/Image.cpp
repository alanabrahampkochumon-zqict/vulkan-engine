/**
 * @file Image.cpp
 * @author Alan Abraham P Kochumon
 * @date Created on: September 29, 2026
 *
 * @brief Implementation of member function declared in Image.h
 *
 * @copyright Copyright (c) 2026 Alan Abraham P Kochumon
 */

#include "Image.h"

#include "../utils/Logger.h"
#include "Buffer.h"
#include "CommandBuffer.h"

namespace tempest::renderer
{

    Image::Image(const RenderDevice& device, const uint32_t width, const uint32_t height, const vk::Format format,
                 const vk::ImageTiling tiling, const vk::ImageUsageFlags usage,
                 const vk::MemoryPropertyFlags properties, const uint32_t mipLevels,
                 const vk::SampleCountFlagBits numSamples, const vk::ImageAspectFlags aspectFlags) noexcept
        : _format{ format },
          _tiling{ tiling },
          _usageFlags{ usage },
          _memoryFlags{ properties },
          _samples{ numSamples },
          _aspectFlags{ aspectFlags },
          _mipLevels{ mipLevels },
          _width{ width },
          _height{ height }
    {
        createImage(device);
        createImageView(device, _aspectFlags);
    }


    void Image::transitionImageLayout(const vk::ImageLayout newLayout, const CommandBuffer& commandBuffer,
                                      const size_t commandBufferIndex) const noexcept
    {
        // To transition an image layout, we need to create a pipeline barrier
        // This can be used for transitioning queue families when vk::SharingMode::eExclusive is used.
        vk::ImageMemoryBarrier barrier{ .oldLayout           = _layout,
                                        .newLayout           = newLayout,
                                        .srcQueueFamilyIndex = vk::QueueFamilyIgnored,
                                        .dstQueueFamilyIndex = vk::QueueFamilyIgnored,
                                        .image               = _image,
                                        // Specify the affect part of the image
                                        .subresourceRange = {
                                            .aspectMask = _aspectFlags, .levelCount = _mipLevels, .layerCount = 1 } };

        // SrcStage -> PipelineBarrier -> DstStage
        vk::PipelineStageFlags sourceStage, destinationStage;
        // We need to handle two transitions Undefined -> TransferOpt and TransferOpt -> ShaderOpt
        // When transitioning from Undefined to ShaderOptimal
        // we are going through TopOfPipe -> Transfer -> FragmentShader
        // Moreover, transfer is a pseudo stage with compute and graphics pipeline.
        // TODO: Make this more generic.
        if (_layout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal)
        {
            barrier.srcAccessMask = {};
            barrier.dstAccessMask = {};

            sourceStage      = vk::PipelineStageFlagBits::eTopOfPipe;
            destinationStage = vk::PipelineStageFlagBits::eTransfer;
        }
        else if (_layout == vk::ImageLayout::eTransferDstOptimal &&
                 newLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
        {
            barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
            barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

            sourceStage      = vk::PipelineStageFlagBits::eTransfer;
            destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
        }
        else
        {
            log::error("Unsupported layout transition");
            return;
        }
        commandBuffer.getBaseCommandBuffers()[commandBufferIndex].pipelineBarrier(sourceStage, destinationStage, {}, {},
                                                                                  nullptr, barrier);
    }


    void Image::copyFromBuffer(const Buffer& buffer, const CommandBuffer& commandBuffer,
                               const size_t commandBufferIndex) const noexcept
    {
        {
            // We need to specify which part of the buffer will be copied to which part of the image.
            const vk::BufferImageCopy region{ .bufferOffset = 0,
                                              // Specify that our image is tightly packed
                                              .bufferRowLength   = 0,
                                              .bufferImageHeight = 0,
                                              .imageSubresource  = { .aspectMask     = vk::ImageAspectFlagBits::eColor,
                                                                     .mipLevel       = 0,
                                                                     .baseArrayLayer = 0,
                                                                     .layerCount     = 1 },
                                              .imageOffset       = { .x = 0, .y = 0, .z = 0 },
                                              .imageExtent       = { .width = _width, .height = _height, .depth = 1 } };
            // Layout indicate the layout the image is currently using.
            // Copy to many images from the buffer is possible.
            commandBuffer.getBaseCommandBuffers()[commandBufferIndex].copyBufferToImage(
                buffer.getBaseBuffer(), _image, vk::ImageLayout::eTransferDstOptimal, region);
        }
    }


    void Image::generateMipmaps(const RenderDevice& device, const CommandBuffer& commandBuffer,
                                const size_t commandBufferIndex) const noexcept
    {
        // Check for bit image platform support
        const auto formatProperties = device.getPhysicalDevice().getFormatProperties(_format);
        if (!(formatProperties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eSampledImageFilterLinear))
        {
            log::error("Texture Image Format doesn't support linear bliting");
            return;
        }

        vk::ImageMemoryBarrier barrier = {
            .srcAccessMask       = vk::AccessFlagBits::eTransferWrite,
            .dstAccessMask       = vk::AccessFlagBits::eTransferRead,
            .oldLayout           = vk::ImageLayout::eTransferDstOptimal,
            .newLayout           = vk::ImageLayout::eTransferSrcOptimal,
            .srcQueueFamilyIndex = vk::QueueFamilyIgnored,
            .dstQueueFamilyIndex = vk::QueueFamilyIgnored,
            .image               = _image,
            .subresourceRange    = { .aspectMask = vk::ImageAspectFlagBits::eColor, .levelCount = 1, .layerCount = 1 }
        };

        int32_t mipWidth = _width, mipHeight = _height;

        for (uint32_t i = 1; i < _mipLevels; ++i)
        {
            // Transfer the barrier to transfer optimal layout
            barrier.subresourceRange.baseMipLevel = i - 1;
            barrier.srcAccessMask                 = vk::AccessFlagBits::eTransferWrite;
            barrier.dstAccessMask                 = vk::AccessFlagBits::eTransferRead;
            barrier.oldLayout                     = vk::ImageLayout::eTransferDstOptimal;
            barrier.newLayout                     = vk::ImageLayout::eTransferSrcOptimal;
            commandBuffer.getBaseCommandBuffers()[commandBufferIndex].pipelineBarrier(
                vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer, {}, {}, {}, barrier);

            // We need specify that we need to blit from mipmap level i-1 to i with half width and height
            vk::ImageBlit blit = {
                .srcSubresource = { .aspectMask = vk::ImageAspectFlagBits::eColor, .mipLevel = i - 1, .layerCount = 1 },
                .srcOffsets     = std::array<vk::Offset3D, 2>({ {}, { mipWidth, mipHeight, 1 } }),
                .dstSubresource = { .aspectMask = vk::ImageAspectFlagBits::eColor, .mipLevel = i, .layerCount = 1 },
                .dstOffsets     = std::array<vk::Offset3D, 2>(
                    { {}, { 1 < mipWidth ? mipWidth / 2 : 1, 1 < mipHeight ? mipHeight / 2 : 1, 1 } })
            };
            // Record the blit command
            commandBuffer.getBaseCommandBuffers()[commandBufferIndex].blitImage(
                _image, vk::ImageLayout::eTransferSrcOptimal, _image, vk::ImageLayout::eTransferDstOptimal, blit,
                vk::Filter::eLinear);
            // Transition the image layout to enable sampling
            barrier.oldLayout     = vk::ImageLayout::eTransferSrcOptimal;
            barrier.newLayout     = vk::ImageLayout::eShaderReadOnlyOptimal;
            barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
            barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
            commandBuffer.getBaseCommandBuffers()[commandBufferIndex].pipelineBarrier(
                vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, {}, {}, {}, barrier);
            // Update the mip width for next mipmap generation
            mipWidth  = mipWidth > 1 ? mipWidth / 2 : 1;
            mipHeight = mipHeight > 1 ? mipHeight / 2 : 1;
        }

        // Transition last mipmap to eShaderReadonly optimal
        barrier.subresourceRange.baseMipLevel = _mipLevels - 1;
        barrier.oldLayout                     = vk::ImageLayout::eTransferDstOptimal;
        barrier.newLayout                     = vk::ImageLayout::eShaderReadOnlyOptimal;
        barrier.srcAccessMask                 = vk::AccessFlagBits::eTransferWrite;
        barrier.dstAccessMask                 = vk::AccessFlagBits::eShaderRead;
        commandBuffer.getBaseCommandBuffers()[commandBufferIndex].pipelineBarrier(
            vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, {}, {}, {}, barrier);
    }



    void Image::createImage(const RenderDevice& device) noexcept
    {
        const vk::ImageCreateInfo imageInfo{ .imageType   = vk::ImageType::e2D,
                                             .format      = _format,
                                             .extent      = { .width = _width, .height = _height, .depth = 1 },
                                             .mipLevels   = _mipLevels,
                                             .arrayLayers = 1,
                                             .samples     = _samples,
                                             .tiling      = _tiling,
                                             .usage       = _usageFlags,
                                             .sharingMode = vk::SharingMode::eExclusive };

        _image = vk::raii::Image(device.getBaseDevice(), imageInfo);

        const vk::MemoryRequirements memRequirements = _image.getMemoryRequirements();
        const vk::MemoryAllocateInfo allocInfo{ .allocationSize  = memRequirements.size,
                                                .memoryTypeIndex = device.findMemoryType(memRequirements.memoryTypeBits,
                                                                                         _memoryFlags) };
        _memory = std::move(vk::raii::DeviceMemory(device.getBaseDevice(), allocInfo));
        _image.bindMemory(_memory, 0);
    }


    void Image::createImageView(const RenderDevice& device, const vk::ImageAspectFlags aspectFlags) noexcept
    {
        const vk::ImageViewCreateInfo viewInfo{ .image            = _image,
                                                .viewType         = vk::ImageViewType::e2D,
                                                .format           = _format,
                                                .subresourceRange = { .aspectMask     = aspectFlags,
                                                                      .baseMipLevel   = 0,
                                                                      .levelCount     = _mipLevels,
                                                                      .baseArrayLayer = 0,
                                                                      .layerCount     = 1 } };

        _imageView = vk::raii::ImageView(device.getBaseDevice(), viewInfo);
    }
} // namespace tempest::renderer
