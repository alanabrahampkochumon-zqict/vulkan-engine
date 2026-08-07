/**
 * @file VulkanEngine.cpp
 * @author Alan Abraham P Kochumon
 * @date Created on: August 7, 2026
 *
 * @brief Implementation of declarations in VulkanRenderer.cppm
 *
 * @copyright Copyright (c) 2026 Alan Abraham P Kochumon
 */

module Engine;

namespace engine
{
    constexpr void VulkanEngine::init() { std::println("Starting engine"); }
    constexpr void VulkanEngine::run() { std::println("Engine is running"); }
    constexpr void VulkanEngine::cleanup() { std::println("Engine is closing..."); }
} // namespace engine
