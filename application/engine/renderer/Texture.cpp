/**
 * @file Texture.cpp
 * @author Alan Abraham P Kochumon
 * @date Created on: September 30, 2026
 *
 * @brief Implementation of functions declared in Texture.h
 *
 * @copyright Copyright (c) 2026 Alan Abraham P Kochumon
 */

#include "Texture.h"

#include "../utils/Logger.h"

#define STB_IMAGE_IMPLEMENTATION
#include "Buffer.h"

#include <stb_image.h>

namespace tempest::renderer
{
    // TODO: Figure out a way to not create two images to make use of it.
    Texture::Texture(RenderDevice& device, std::string texturePath) noexcept
        : _image{}, _sampler{ nullptr }, _path{ std::move(texturePath) }, _device{ device }
    {
        stbi_uc* pixels          = stbi_load(_path.c_str(), &_width, &_height, &_channels, STBI_rgb_alpha);
        vk::DeviceSize imageSize = _width * _height * 4;

        if (!pixels)
        {
            log::error("Failed to load texture image");
            return;
        }

        // Calculate texture mipmap levels
        const auto mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(_width, _height)))) + 1;


        constexpr auto properties =
            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
        // Move the image to staging buffer
        Buffer stagingBuffer(_device);
        stagingBuffer.create(imageSize, vk::BufferUsageFlagBits::eTransferSrc, properties);
        stagingBuffer.memcpy(pixels, imageSize);

        // Free the image buffer
        stbi_image_free(pixels);

        _image = std::move(Image(_device, static_cast<uint32_t>(_width), static_cast<uint32_t>(_height),
                                 vk::Format::eR8G8B8A8Srgb, vk::ImageTiling::eOptimal,
                                 vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc |
                                     vk::ImageUsageFlagBits::eSampled,
                                 vk::MemoryPropertyFlagBits::eDeviceLocal, mipLevels, vk::SampleCountFlagBits::e1,
                                 vk::ImageAspectFlagBits::eColor));

        const CommandBuffer commandBuffer{ _device, _device.getQueues()[0], 1 };
        commandBuffer.beginSingleTimeRecording();
        // Transition the image from undefined to transfer optimal layout
        _image.transitionImageLayout(vk::ImageLayout::eTransferDstOptimal, commandBuffer);
        // Copy the image from staging buffer to textureImage buffer
        _image.copyFromBuffer(stagingBuffer, commandBuffer, 1);
        // Transition to a layout optimal for sampling(while generating mipmaps)
        _image.generateMipmaps(_device, commandBuffer);
        // transitionImageLayout(commandBuffer, textureImage, vk::ImageLayout::eTransferDstOptimal,
        //                       vk::ImageLayout::eShaderReadOnlyOptimal, textureMipmapLevels);
        commandBuffer.endSingleTimeRecordingAndSubmit();

        createSampler();
    }


    void Texture::createSampler() noexcept
    {
        const vk::PhysicalDeviceProperties properties = _device.getPhysicalDevice().getProperties();
        vk::SamplerCreateInfo samplerInfo{ .magFilter    = vk::Filter::eLinear,
                                           .minFilter    = vk::Filter::eLinear,
                                           .mipmapMode   = vk::SamplerMipmapMode::eLinear,
                                           .addressModeU = vk::SamplerAddressMode::eRepeat,
                                           .addressModeV = vk::SamplerAddressMode::eRepeat,
                                           .addressModeW = vk::SamplerAddressMode::eRepeat,
                                           .mipLodBias   = 0.0f,
                                           // Anisotropic filtering
                                           .anisotropyEnable = vk::True,
                                           .maxAnisotropy    = properties.limits.maxSamplerAnisotropy,
                                           // If comparison is enabled then texels will be compared to a value.
                                           .compareEnable = vk::True,
                                           .compareOp     = vk::CompareOp::eAlways,
                                           .minLod        = 0.0f,
                                           .maxLod        = vk::LodClampNone };
        // What color to use beyond the clamp. Cannot specify an arbitrary color.
        samplerInfo.borderColor = vk::BorderColor::eFloatOpaqueBlack;
        // Unnormalized coordinates allows us to sample beyond the [0,1) range.
        samplerInfo.unnormalizedCoordinates = vk::False;
        // Map mapping
        samplerInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;

        _sampler = vk::raii::Sampler(_device.getBaseDevice(), samplerInfo);
    }

} // namespace tempest::renderer
