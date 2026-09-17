module;
#include <SDL3/sdl.h>
#include <string>

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include "../../out/build/MSVC-Release/_deps/glm-src/glm/ext/scalar_uint_sized.hpp"

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

export module TempestEngine;
export import :UBO;
export import :Vertex;

namespace tempest
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
        void drawFrame();
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
        void createDescriptorSetLayout();
        void createGraphicsPipeline();
        void createVertexBuffer();
        void createIndexBuffer();
        void createUniformBuffers();
        void createDescriptorPool();
        void createDescriptorSets();
        void createTextureImage();
        void createTextureImageView();
        void createDepthResources();
        void createColorResources();
        vk::raii::ImageView createImageView(const vk::Image& image, vk::Format format, vk::ImageAspectFlags aspectFlags,
                                            uint32_t mipLevel) const;
        [[nodiscard]] vk::raii::ShaderModule createShaderModule(const std::vector<char>& code) const;

        vk::SurfaceFormatKHR chooseSurfaceFormat(
            const std::vector<vk::SurfaceFormatKHR>& surfaceFormats) const noexcept;
        vk::PresentModeKHR choosePresentationMode(const std::vector<vk::PresentModeKHR>& presentModes) const noexcept;
        vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities) const noexcept;
        uint32_t chooseMinImageCount(const vk::SurfaceCapabilitiesKHR& capabilities) const noexcept;
        void createCommandPool() noexcept;
        void createCommandBuffer() noexcept;
        void createSyncObjects() noexcept;
        void createTextureSampler() noexcept;
        std::pair<vk::raii::Image, vk::raii::DeviceMemory> createImage(
            uint32_t width, uint32_t height, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage,
            vk::MemoryPropertyFlags properties, uint32_t mipLevels, vk::SampleCountFlagBits numSamples) const noexcept;
        void recordCommandBuffer(uint32_t imageIndex) const noexcept;
        // Transition an image layout for rendering, submission etc.
        void transitionImageLayout(vk::Image image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout,
                                   vk::AccessFlags2 srcAccessMask, vk::AccessFlags2 dstAccessMask,
                                   vk::PipelineStageFlags2 srcStageMask, vk::PipelineStageFlags2 dstStageMask,
                                   vk::ImageAspectFlags aspectFlags) const noexcept;
        void transitionImageLayout(vk::raii::CommandBuffer& commandBuffer, const vk::raii::Image& image,
                                   vk::ImageLayout oldLayout, vk::ImageLayout newLayout, uint32_t mipLevels);
        void copyBufferToImage(vk::raii::CommandBuffer& commandBuffer, const vk::raii::Buffer& buffer,
                               vk::raii::Image& image, uint32_t width, uint32_t height) noexcept;
        std::pair<vk::raii::Buffer, vk::raii::DeviceMemory> createBuffer(
            vk::DeviceSize size, vk::BufferUsageFlags usageFlags, vk::MemoryPropertyFlags properties) const noexcept;
        void copyBuffer(const vk::raii::Buffer& srcBuffer, const vk::raii::Buffer& dstBuffer,
                        vk::DeviceSize bufferSize) const noexcept;
        void updateUniformBuffer(uint32_t currentImageIdx) noexcept;
        vk::raii::CommandBuffer beginSingleTimeCommands() const noexcept;
        void endSingleTimeCommands(const vk::raii::CommandBuffer&& commandBuffer) const noexcept;
        [[nodiscard]] uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) const;
        [[nodiscard]] vk::Format findSupportedFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling,
                                                     vk::FormatFeatureFlags features) const;
        [[nodiscard]] vk::Format findDepthFormat() const;
        void recreateSwapChain();
        void cleanupSwapChain();
        void loadModel();
        void generateMipmaps(const vk::raii::CommandBuffer& commandBuffer, const vk::raii::Image& image,
                             vk::Format imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);
        vk::SampleCountFlagBits getMaxUsableSampleCount() noexcept;


        std::string appName{}, appVersion{}, appId{};
        size_t width{}, height{};

        SDL_Window* window{ nullptr };

        static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

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
        vk::raii::DescriptorSetLayout descriptorSetLayout{ nullptr };
        vk::raii::PipelineLayout pipelineLayout{ nullptr };
        vk::raii::Pipeline graphicsPipeline{ nullptr };
        vk::raii::CommandPool commandPool{ nullptr };
        vk::raii::Buffer vertexBuffer{ nullptr }, indexBuffer{ nullptr };
        vk::raii::DeviceMemory vertexBufferMemory{ nullptr }, indexBufferMemory{ nullptr };
        std::vector<vk::raii::Buffer> uniformBuffers{};
        std::vector<vk::raii::DeviceMemory> uniformBuffersMemory{};
        std::vector<void*> uniformBuffersMapped{};
        std::vector<vk::raii::CommandBuffer> commandBuffers{};
        std::vector<vk::raii::Semaphore> renderFinishedSemaphores{}, presentFinishedSemaphores{};
        vk::raii::DescriptorPool descriptorPool{ nullptr };
        std::vector<vk::raii::DescriptorSet> descriptorSets{};
        vk::raii::Image textureImage{ nullptr };
        vk::raii::DeviceMemory textureImageMemory{ nullptr };
        vk::raii::ImageView textureImageView{ nullptr };
        vk::raii::Sampler textureSampler{ nullptr };
        vk::raii::Image depthImage{ nullptr };
        vk::raii::DeviceMemory depthImageMemory{ nullptr };
        vk::raii::ImageView depthImageView{ nullptr };
        uint32_t textureMipmapLevels{ 4 };
        vk::SampleCountFlagBits msaaSamples = vk::SampleCountFlagBits::e1;
        vk::raii::Image colorImage{ nullptr };
        vk::raii::DeviceMemory colorImageMemory{ nullptr };
        vk::raii::ImageView colorImageView{ nullptr };

        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;

        // Fence is required since we don't want to overwrite the currently rendering frame
        std::vector<vk::raii::Fence> drawFences{};
        uint32_t frameIndex = 1;
        uint32_t queueIndex = ~0; // 0b11111...1

        std::string ENGINE_NAME{ "Tempest" };
        bool _isRunning{ false };

        /// CONFIG
        static constexpr uint32_t WIDTH    = 800;
        static constexpr uint32_t HEIGHT   = 600;
        static constexpr auto MODEL_PATH   = "models/viking_room.obj";
        static constexpr auto TEXTURE_PATH = "textures/viking_room.png";


#ifdef NDEBUG
        static constexpr bool enableValidationLayers = false;
#else
        static constexpr bool enableValidationLayers = true;
#endif
    };

} // namespace tempest