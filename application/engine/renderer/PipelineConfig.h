#pragma once
/**
 * @file PipelineConfig.h
 * @author Alan Abraham P Kochumon
 * @date Created on: October 01, 2026
 *
 * @brief Configuration for Render Pipeline.
 *
 * @copyright Copyright (c) 2026 Alan Abraham P Kochumon
 */

#include <string>
#include <vulkan/vulkan.hpp>

namespace tempest::renderer
{
    struct PipelineConfig
    {
        std::string vertPath;
        std::string fragPath;
        std::string vertMainName{ "main" };
        std::string fragMainName{ "main" };
        vk::SampleCountFlagBits sampleCount = vk::SampleCountFlagBits::e1;

        static PipelineConfig defaultConfig() noexcept;
    };


    inline PipelineConfig PipelineConfig::defaultConfig() noexcept
    { return PipelineConfig{ .vertPath = "shaders/basic.vert", .fragPath = "shaders/basic.frag" }; }
} // namespace tempest::renderer