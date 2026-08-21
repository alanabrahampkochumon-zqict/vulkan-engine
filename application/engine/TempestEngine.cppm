module;
#include <SDL3/sdl.h>
#include <string>

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

export module TempestEngine;

namespace engine
{
    export class TempestEngine
    {
    public:
        TempestEngine() = default;
        void init(const std::string& applicationName, const std::string& version, const std::string& id, size_t width,
                  size_t height);
        void run();
        void cleanup() const;

    private:
        void handleEvents();
        void initVulkan();
        static std::vector<const char*> getRequiredExtensions() noexcept;
        static VKAPI_ATTR vk::Bool32 VKAPI_CALL
        debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT severity, vk::DebugUtilsMessageTypeFlagsEXT type,
                      const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
        void createVulkanInstance();
        void setupDebugMessenger();
        void createSurface();
        void pickPhysicalDevice();
        void createLogicalDevice();
        void createSwapChain();
        void createImageViews();

        vk::SurfaceFormatKHR chooseSurfaceFormat(
            const std::vector<vk::SurfaceFormatKHR>& surfaceFormats) const noexcept;
        vk::PresentModeKHR choosePresentationMode(const std::vector<vk::PresentModeKHR>& presentModes) const noexcept;
        vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities) const noexcept;
        uint32_t chooseMinImageCount(const vk::SurfaceCapabilitiesKHR& capabilities) const noexcept;


        std::string appName{}, appVersion{}, appId{};
        size_t width{}, height{};


        SDL_Window* window{ nullptr };

        vk::raii::Context context{};
        vk::raii::Instance instance{ nullptr };
        std::vector<const char*> validationLayers{ "VK_LAYER_KHRONOS_validation" };
        vk::DebugUtilsMessengerEXT debugMessenger{};
        vk::raii::PhysicalDevice physicalDevice{ nullptr };
        vk::raii::Device device{ nullptr };
        vk::raii::Queue graphicsQueue{ nullptr };
        vk::PhysicalDeviceFeatures deviceFeatures{};
        vk::raii::SurfaceKHR surface{ nullptr };
        vk::raii::SwapchainKHR swapChain{ nullptr };
        std::vector<vk::Image> swapChainImages{};
        std::vector<vk::raii::ImageView> swapChainImageViews{};
        vk::Extent2D swapChainExtent;
        vk::SurfaceFormatKHR swapChainSurfaceFormat;

        std::string ENGINE_NAME{ "Tempest" };
        bool _isRunning{ false };


#ifdef NDEBUG
        static constexpr bool enableValidationLayers = false;
#else
        static constexpr bool enableValidationLayers = true;
#endif
    };

} // namespace engine