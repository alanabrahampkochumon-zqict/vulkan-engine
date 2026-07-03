#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <print>
#include <vector>
#include <vulkan/vulkan.h>


static auto APP_NAME    = "Vulkan Renderer";
static auto APP_ID      = "com.alan.vulkan_renderer";
static auto APP_VERSION = "1.0.0";


class HelloTriangleApplication
{
public:
    void run()
    {
        initWindow();
        initVulkan();
        mainLoop();
        cleanUp();
    }

private:
    void initWindow()
    {
        SDL_SetAppMetadata(APP_NAME, APP_VERSION, APP_ID);
        if (!SDL_Init(SDL_INIT_VIDEO))
            throw std::exception("There was an error initializing the Window"); // TODO: Strip down exceptions

        _window = SDL_CreateWindow(APP_NAME, WIDTH, HEIGHT, 0);
    }

    void initVulkan()
    {
        createInstance();
        queryAvailableExtensions();
    }

    void mainLoop()
    {
        while (_gameRunning)
        {
            SDL_Event event;
            while (SDL_PollEvent(&event))
            {
                switch (event.type)
                {
                    case SDL_EVENT_QUIT:
                        _gameRunning = false;
                        break;
                    default:
                        continue;
                }
            }
        }
    }

    void cleanUp()
    {
        vkDestroyInstance(_vkInstance, nullptr);
        SDL_DestroyWindow(_window);
    }

    void createInstance()
    {
// Enable Validation only in Debug mode
#ifdef NDEBUG
        const bool enableValidation = false;
#else
        const bool enableValidation = true;
#endif


        // Optional information added for optimization
        VkApplicationInfo appInfo{};                        // Initializes pNext to nullptr (extension information)
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO; // sType needs to be specified

        // Application Info
        appInfo.pApplicationName   = APP_NAME;
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);

        // Engine Info
        appInfo.pEngineName   = "Vulkan Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);

        // API Info
        appInfo.apiVersion = VK_API_VERSION_1_0;

        // Create an Instance info to specify to the driver which extensions and validation layers to use
        VkInstanceCreateInfo createInfo{};
        createInfo.sType            = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        // Connect SDL3 to Vulkan for creating vkInstance
        uint32_t instanceExtensionCount       = 0;
        const char* const* instanceExtensions = SDL_Vulkan_GetInstanceExtensions(&instanceExtensionCount);

        std::vector<const char*> requiredExtensions;
        for (uint32_t i = 0; i < instanceExtensionCount; ++i)
            requiredExtensions.emplace_back(instanceExtensions[i]);

        if (enableValidation)
        {
            requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        // Required on macOS which will give the `VK_ERROR_INCOMPATIBLE_DRIVER` when creating vkInstance
#if defined(__APPLE__) && defined(__MACH__)
        // MacOS Specific
        requiredExtensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
        createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif

        createInfo.enabledExtensionCount   = static_cast<uint32_t>(requiredExtensions.size());
        createInfo.ppEnabledExtensionNames = requiredExtensions.data();


        //////  Validation Layer Enabling
        std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };

        // Check for validation
        if (enableValidation && !queryExtensionAvailability(validationLayers))
        {
            throw std::runtime_error("Validation layers requested but not supported!");
        }
        if (enableValidation)
        {
            // Enable validation
            createInfo.enabledLayerCount   = validationLayers.size();
            createInfo.ppEnabledLayerNames = validationLayers.data();
        }
        else
        {
            createInfo.enabledLayerCount = 0;
        }
        SDL_Log("Enabled Vulkan Validation Layer!");

        // Create the vulkan instance
        if (vkCreateInstance(&createInfo, nullptr, &_vkInstance) == VK_SUCCESS)
            SDL_Log("Initialized Vulkan!");
        else
            SDL_Log("There was an error creating the Vulkan instance.");
    }

    inline void setupValidationLayer(VkInstanceCreateInfo& instanceInfo) {}

    bool queryExtensionAvailability(const std::vector<const char*>& layers)
    {
        uint32_t numLayers{};
        vkEnumerateInstanceLayerProperties(&numLayers, nullptr);
        std::vector<VkLayerProperties> availableLayers(numLayers);
        vkEnumerateInstanceLayerProperties(&numLayers, availableLayers.data());

        bool layerAvailable = false;

        for (const auto& targetLayer : layers)
        {
            for (const auto& availableLayer : availableLayers)
            {
                if (strcmp(targetLayer, availableLayer.layerName) == 0)
                {
                    layerAvailable = true;
                    break;
                }
            }
            if (!layerAvailable)
                return false;
        }
        return true;
    }

    void queryAvailableExtensions()
    {
        uint32_t numExtensions{};
        // query the number of extensions
        vkEnumerateInstanceExtensionProperties(nullptr, &numExtensions, nullptr);
        std::vector<VkExtensionProperties> extensions(numExtensions);

        // Query the extensions
        vkEnumerateInstanceExtensionProperties(nullptr, &numExtensions, extensions.data());

        std::println("Supported extensions: ");
        for (const auto& [extensionName, specVersion] : extensions)
        {
            std::println("\t{}", extensionName);
        }
    }

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                        VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                        void* pUserData)
    {
        if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
            SDL_Log("[Vulkan Validation]: %s", pCallbackData->pMessage);
        return VK_FALSE;
    }

private:
    SDL_Window* _window{ nullptr };
    static constexpr size_t WIDTH  = 1280;
    static constexpr size_t HEIGHT = 720;

    bool _gameRunning{ true };

    VkInstance _vkInstance{};
};

int main()
{
    HelloTriangleApplication app{};

    try
    {
        app.run();
    }
    catch (const std::exception& e)
    {
        SDL_Log("%s\n", e.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}