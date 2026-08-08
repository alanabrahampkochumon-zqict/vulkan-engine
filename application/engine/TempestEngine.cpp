/**
 * @file VulkanEngine.cpp
 * @author Alan Abraham P Kochumon
 * @date Created on: August 7, 2026
 *
 * @brief Implementation of declarations in VulkanRenderer.cppm
 *
 * @copyright Copyright (c) 2026 Alan Abraham P Kochumon
 */

module;
#include <print>
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan.hpp>

module Engine;


namespace engine
{
    void TempestEngine::init() { std::println("Starting engine"); }
    void TempestEngine::run() { std::println("Engine is running"); }
    void TempestEngine::cleanup() { std::println("Engine is closing..."); }
} // namespace engine
