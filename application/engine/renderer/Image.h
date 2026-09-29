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

#include "RenderDevice.h"

#include <vulkan/vulkan_raii.hpp>

namespace tempest::renderer
{
    class Image
    {
    public:
        Image(const RenderDevice& device, uint32_t width, uint32_t height, vk::Format format, vk::ImageTiling tiling,
              vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, uint32_t mipLevels,
              vk::SampleCountFlagBits numSamples,
              vk::ImageAspectFlags aspectFlags) noexcept;

        [[nodiscard]] const vk::raii::Image& getBaseImage() const { return _image; }
        [[nodiscard]] const vk::raii::ImageView& getBaseImageView() const { return _imageView; }


    private:
        void createImage() noexcept;
        void createImageView(vk::ImageAspectFlags aspectFlags) noexcept;

    private:
        const RenderDevice& _device;
        vk::raii::Image _image{ nullptr };
        vk::raii::ImageView _imageView{ nullptr };
        vk::raii::DeviceMemory _memory{ nullptr };
        vk::Format _format;
        vk::ImageTiling _tiling;
        vk::ImageUsageFlags _usageFlags;
        vk::MemoryPropertyFlags _memoryFlags;
        vk::SampleCountFlagBits _samples;
        uint32_t _mipLevels;
        uint32_t _width, _height;
    };
} // namespace tempest::renderer