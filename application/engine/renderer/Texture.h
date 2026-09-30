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
        Texture(std::string texturePath) noexcept;


    private:
        void loadTexture() noexcept;


    private:
        Image _image;
        vk::raii::Sampler _sampler;
        std::string _path;
    };
} // namespace tempest::renderer