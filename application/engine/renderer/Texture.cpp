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

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace tempest::renderer
{
    Texture::Texture(const RenderDevice& device, std::string texturePath) noexcept: _path{ std::move(texturePath) }
    { _image = Image(device, ); }

    void Texture::loadTexture() noexcept
    {
        // Read the image
        // int texWidth, texHeight, texChannels;
        // stbi_uc* pixels          = stbi_load(TEXTURE_PATH, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        // vk::DeviceSize imageSize = texWidth * texHeight * 4;
        //
        // if (!pixels)
        // {
        //     throw std::runtime_error("Failed to load texture image");
        // }
        //
        // // Calculate texture mipmap levels
        // textureMipmapLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;
        //
        //
        // constexpr auto properties =
        //     vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
        // // Move the image to staging buffer
        // const auto& [stagingBuffer, stagingBufferMemory] =
        //     createBuffer(imageSize, vk::BufferUsageFlagBits::eTransferSrc, properties);
        // void* data = stagingBufferMemory.mapMemory(0, imageSize);
        // memcpy(data, pixels, imageSize);
        // stagingBufferMemory.unmapMemory();
        //
        // // Free the image buffer
        // stbi_image_free(pixels);
        //
        // // When generating mipmaps we need the image to a transfer, so we add transfer dst, src, and sample flags.
        // std::tie(textureImage, textureImageMemory) =
        //     createImage(texWidth, texHeight, vk::Format::eR8G8B8A8Srgb, vk::ImageTiling::eOptimal,
        //                 vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc |
        //                     vk::ImageUsageFlagBits::eSampled,
        //                 vk::MemoryPropertyFlagBits::eDeviceLocal, textureMipmapLevels, vk::SampleCountFlagBits::e1);
        // vk::raii::CommandBuffer commandBuffer = beginSingleTimeCommands();
        // // Transition the image from undefined to transfer optimal layout
        // transitionImageLayout(commandBuffer, textureImage, vk::ImageLayout::eUndefined,
        //                       vk::ImageLayout::eTransferDstOptimal, textureMipmapLevels);
        // // Copy the image from staging buffer to textureImage buffer
        // copyBufferToImage(commandBuffer, stagingBuffer, textureImage, static_cast<uint32_t>(texWidth),
        //                   static_cast<uint32_t>(texHeight));
        // // Transition to a layout optimal for sampling(while generating mipmaps)
        // generateMipmaps(commandBuffer, textureImage, vk::Format::eR8G8B8A8Srgb, texWidth, texHeight,
        //                 textureMipmapLevels);
        // // transitionImageLayout(commandBuffer, textureImage, vk::ImageLayout::eTransferDstOptimal,
        // //                       vk::ImageLayout::eShaderReadOnlyOptimal, textureMipmapLevels);
        // // End the command
        // endSingleTimeCommands(std::move(commandBuffer));
    }
} // namespace tempest::renderer
