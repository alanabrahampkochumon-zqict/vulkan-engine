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

#include "Buffer.h"

namespace tempest::renderer
{

    Image::Image(const RenderDevice& device, const uint32_t width, const uint32_t height, const vk::Format format,
                 const vk::ImageTiling tiling, const vk::ImageUsageFlags usage,
                 const vk::MemoryPropertyFlags properties, const uint32_t mipLevels,
                 const vk::SampleCountFlagBits numSamples, const vk::ImageAspectFlags aspectFlags) noexcept
        : _device{ device },
          _format{ format },
          _tiling{ tiling },
          _usageFlags{ usage },
          _memoryFlags{ properties },
          _samples{ numSamples },
          _mipLevels{ mipLevels },
          _width{ width },
          _height{ height }
    {
        createImage();
        createImageView(aspectFlags);
    }


    void Image::createImage() noexcept
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

        _image = vk::raii::Image(_device.getDevice(), imageInfo);

        const vk::MemoryRequirements memRequirements = _image.getMemoryRequirements();
        const vk::MemoryAllocateInfo allocInfo{ .allocationSize  = memRequirements.size,
                                                .memoryTypeIndex = Buffer::findMemoryType(
                                                    _device, memRequirements.memoryTypeBits, _memoryFlags) };
        _memory = std::move(vk::raii::DeviceMemory(_device.getDevice(), allocInfo));
        _image.bindMemory(_memory, 0);
    }

    void Image::createImageView(const vk::ImageAspectFlags aspectFlags) noexcept
    {
        const vk::ImageViewCreateInfo viewInfo{ .image            = _image,
                                                .viewType         = vk::ImageViewType::e2D,
                                                .format           = _format,
                                                .subresourceRange = { .aspectMask     = aspectFlags,
                                                                      .baseMipLevel   = 0,
                                                                      .levelCount     = _mipLevels,
                                                                      .baseArrayLayer = 0,
                                                                      .layerCount     = 1 } };

        _imageView = vk::raii::ImageView(_device.getDevice(), viewInfo);
    }
} // namespace tempest::renderer
