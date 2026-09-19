module;
/**
 * @file RendererContext.cppm
 * @author Alan Abraham P Kochumon
 * @date Created on: September 19, 2026
 *
 * @brief Defines the vulkan rendering context.
 *
 * @copyright Copyright (c) 2026 Alan Abraham P Kochumon
 */

#include <functional>
#include <string>
#include <vulkan/vulkan_raii.hpp>

export module TempestEngine:Renderer;



namespace tempest
{
    class RenderingContext
    {
        // Create a vulkan instance, device, logical device
    public:
        constexpr RenderingContext(std::string applicationName, uint32_t applicationVersion,
                                   const std::vector<const char*>& extensions, bool enableValidationLayers = true,
                                   std::string engineName = "Tempest", uint32_t engineVersion = 1) noexcept;

        ///
        // [[nodiscard]] constexpr const vk::raii::Context& getVulkanContext() const noexcept;

        /// Get the rendering context's vulkan instance.
        [[nodiscard]] constexpr const vk::raii::Instance& getVulkanInstance() const noexcept;

        void attachLogger(const std::function<void(std::string)>& loggingFunc) noexcept;

    private:
        constexpr bool instantiateVkInstance() noexcept;
        constexpr bool pickPhysicalDevice() noexcept;
        constexpr bool createLogicalDevice() noexcept;

    private:
        vk::raii::Context _vulkanContext{};
        vk::raii::Instance _vulkanInstance{ nullptr };
        vk::raii::PhysicalDevice _selectedGPU{ nullptr };
        vk::raii::Device _logicalDevice{ nullptr };
        vk::raii::Queue _graphicsQueue{ nullptr };

        std::function<void(std::string)> _log;

        std::string _engineName, _applicationName;
        uint32_t _appVersion, _engineVersion;
        bool _enableValidationLayers;


        std::vector<const char*> _validationLayers{ "VK_LAYER_KHRONOS_validation" };
        std::vector<const char*> _extensions;
    };

} // namespace tempest
