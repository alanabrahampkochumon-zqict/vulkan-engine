#pragma once
/**
 * @file Texture.h
 * @author Alan Abraham P Kochumon
 * @date Created on: September 30, 2026
 *
 * @brief Texture abstraction.
 *
 * @copyright Copyright (c) 2026 Alan Abraham P Kochumon
 */

#include "Image.h"

#include <vulkan/vulkan_raii.hpp>


namespace tempest::renderer
{

    class Texture
    {
    public:
        Texture(RenderDevice& device, std::string texturePath) noexcept;

        [[nodiscard]] const Image& getImage() const { return _image; }
        [[nodiscard]] const vk::raii::Sampler& getSampler() const { return _sampler; }
        [[nodiscard]] int getWidth() const { return _width; }
        [[nodiscard]] int getHeight() const { return _height; }
        [[nodiscard]] int getChannels() const { return _channels; }

    private:
        void createSampler() noexcept;


    private:
        Image _image;
        vk::raii::Sampler _sampler;
        std::string _path;
        int _width, _height, _channels;
        RenderDevice& _device;
    };
} // namespace tempest::renderer