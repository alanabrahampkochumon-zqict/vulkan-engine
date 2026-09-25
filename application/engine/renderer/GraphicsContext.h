#pragma once
/**
 * @file GraphicsContext.h
 * @author Alan Abraham P Kochumon
 * @date Created on: September 25, 2026
 *
 * @brief Encapsulates graphics context for the rendering API.
 *
 * @copyright Copyright (c) 2026 Alan Abraham P Kochumon
 */

// #define VULKAN_HPP_NO_EXCEPTIONS // TODO: Look into this before adding

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

namespace tempest::renderer
{
    class GraphicsContext
    {

        GraphicsContext(const std::string& applicationName, uint32_t appVersion, const std::string& engineName,
                        uint32_t engineVersion, bool enableValidationLayers = true,
                        uint32_t minAPIVersion = MIN_API_VERSION) noexcept;

    public:
        GraphicsContext(const GraphicsContext& other)            = delete;
        GraphicsContext& operator=(const GraphicsContext& other) = delete;

        GraphicsContext(GraphicsContext&& other) noexcept;
        GraphicsContext& operator=(GraphicsContext&& other) noexcept;


        [[nodiscard]] const vk::raii::Context& getBaseContext() const { return _context; }
        [[nodiscard]] const vk::raii::Instance& getInstance() const { return _instance; }

    private:
        /// Vulkan Debug Callback API
        static VKAPI_ATTR vk::Bool32 VKAPI_CALL
        debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT severity, vk::DebugUtilsMessageTypeFlagsEXT type,
                      const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);

        bool createVulkanInstance(const std::string& applicationName, uint32_t appVersion,
                                  const std::string& engineName, uint32_t engineVersion,
                                  uint32_t minAPIVersion) noexcept;
        void setupDebugMessenger() noexcept;

        std::vector<const char*> getRequiredExtensions() const noexcept;

    private:
        vk::raii::Context _context{};
        vk::raii::Instance _instance{ nullptr };
        vk::DebugUtilsMessengerEXT _debugMessenger{};
        bool _enableValidationLayers;
        std::vector<const char*> _validationLayers{ "VK_LAYER_KHRONOS_validation" };

        static constexpr uint32_t MIN_API_VERSION{ vk::ApiVersion13 };
    };
} // namespace tempest::renderer
