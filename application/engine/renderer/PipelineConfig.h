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
        std::string vertMainName;
        std::string fragPath;
        std::string fragMainName;
        vk::SampleCountFlagBits sampleCount = vk::SampleCountFlagBits::e1;
    };
} // namespace tempest::renderer