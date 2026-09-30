#pragma once
/**
 * @file Image.h
 * @author Alan Abraham P Kochumon
 * @date Created on: September 29, 2026
 *
 * @brief Abstraction for Rendering Image and ImageView.
 *
 * @copyright Copyright (c) 2026 Alan Abraham P Kochumon
 */

#include "Buffer.h"
#include "CommandBuffer.h"
#include "RenderDevice.h"

#include <vulkan/vulkan_raii.hpp>

namespace tempest::renderer
{
    class Image
    {
    public:
        Image(RenderDevice& device, uint32_t width, uint32_t height, vk::Format format, vk::ImageTiling tiling,
              vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, uint32_t mipLevels,
              vk::SampleCountFlagBits numSamples, vk::ImageAspectFlags aspectFlags) noexcept;

        Image(const Image& other)            = delete;
        Image& operator=(const Image& other) = delete;

        Image(Image&& other) noexcept
            : _device{ other._device },
              _image{ std::move(other._image) },
              _imageView{ std::move(other._imageView) },
              _memory{ std::move(other._memory) },
              _layout{ other._layout },
              _format{ other._format },
              _tiling{ other._tiling },
              _usageFlags{ std::move(other._usageFlags) },
              _memoryFlags{ std::move(other._memoryFlags) },
              _samples{ other._samples },
              _aspectFlags{ std::move(other._aspectFlags) },
              _mipLevels{ other._mipLevels },
              _width{ other._width },
              _height{ other._height }
        {}

        Image& operator=(Image&& other) noexcept
        {
            if (this == &other)
                return *this;
            _device      = std::move(other._device);
            _image       = std::move(other._image);
            _imageView   = std::move(other._imageView);
            _memory      = std::move(other._memory);
            _layout      = other._layout;
            _format      = other._format;
            _tiling      = other._tiling;
            _usageFlags  = std::move(other._usageFlags);
            _memoryFlags = std::move(other._memoryFlags);
            _samples     = other._samples;
            _aspectFlags = std::move(other._aspectFlags);
            _mipLevels   = other._mipLevels;
            _width       = other._width;
            _height      = other._height;
            return *this;
        }

        [[nodiscard]] const vk::raii::Image& getBaseImage() const noexcept { return _image; }
        [[nodiscard]] const vk::raii::ImageView& getBaseImageView() const noexcept { return _imageView; }
        [[nodiscard]] const vk::ImageLayout& getImageLayout() const noexcept { return _layout; }

        void transitionImageLayout(vk::ImageLayout newLayout, const CommandBuffer& commandBuffer,
                                   size_t commandBufferIndex = 0) const noexcept;

        /// Copy data from a staging buffer to this image.
        void copyFromBuffer(const Buffer& buffer, const CommandBuffer& commandBuffer,
                            size_t commandBufferIndex = 1) const noexcept;

        void generateMipmaps(const CommandBuffer& commandBuffer, size_t commandBufferIndex = 0) const noexcept;

    private:
        explicit Image(RenderDevice& _device) noexcept
            : _device{ _device },
              _format{ vk::Format::eR8G8B8A8Srgb },
              _tiling{ vk::ImageTiling::eOptimal },
              _samples{ vk::SampleCountFlagBits::e1 }
        {}
        void createImage() noexcept;
        void createImageView(vk::ImageAspectFlags aspectFlags) noexcept;

    private:
        RenderDevice& _device;
        vk::raii::Image _image{ nullptr };
        vk::raii::ImageView _imageView{ nullptr };
        vk::raii::DeviceMemory _memory{ nullptr };
        vk::ImageLayout _layout{ vk::ImageLayout::eUndefined };
        vk::Format _format;
        vk::ImageTiling _tiling;
        vk::ImageUsageFlags _usageFlags;
        vk::MemoryPropertyFlags _memoryFlags;
        vk::SampleCountFlagBits _samples;
        vk::ImageAspectFlags _aspectFlags;
        uint32_t _mipLevels{ 0 };
        uint32_t _width{ 0 }, _height{ 0 };

        friend class Texture;
    };
} // namespace tempest::renderer