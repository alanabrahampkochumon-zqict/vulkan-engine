#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <map>
#include <optional>
#include <print>
#include <vector>
#include <vulkan/vulkan.h>


static auto APP_NAME    = "Vulkan Renderer";
static auto APP_ID      = "com.alan.vulkan_renderer";
static auto APP_VERSION = "1.0.0";

struct QueueFamilyIndices
{
    std::optional<uint32_t> graphicsFamily{ std::nullopt };

    /**
     * @brief Returns whether the current queue family has a device feature set.
     */
    bool isComplete() { return graphicsFamily.has_value(); }
};


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
        setupDebugMessenger();
        pickPhysicalDevice();
        createLogicalDevice();
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
        vkDestroyDevice(_vkDevice, nullptr);

        if (_enabledValidationLayers)
            DestroyDebugUtilsMessengerEXT(_vkInstance, _debugMessenger, nullptr);

        vkDestroyInstance(_vkInstance, nullptr);
        SDL_DestroyWindow(_window);
    }

    void createInstance()
    {
// Enable Validation only in Debug mode
#ifdef NDEBUG
        const bool enableValidation = false;
        _enabledValidationLayers    = false;
#else
        const bool enableValidation = true;
        _enabledValidationLayers    = true;
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
        if (enableValidation && !queryLayerAvailability(validationLayers))
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

    void setupDebugMessenger()
    {
        if (!_enabledValidationLayers)
            return;

        // Create messenger create info
        VkDebugUtilsMessengerCreateInfoEXT messengerCreateInfo;
        messengerCreateInfo.sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        messengerCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        messengerCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        messengerCreateInfo.pfnUserCallback = debugCallback;
        messengerCreateInfo.flags           = 0;       // Flags must be zero
        messengerCreateInfo.pUserData       = nullptr; // User data

        if (CreateDebugUtilsMessengerEXT(_vkInstance, &messengerCreateInfo, nullptr, &_debugMessenger) != VK_SUCCESS)
            throw std::runtime_error("Failed to set up debug messenger");
    }

    void createLogicalDevice()
    {
        QueueFamilyIndices familyIndices = findQueueFamilies(_physicalDevice);
        float queuePriority              = 1.0f; // Must specify a priority even if its the one queue

        // Used Device features
        VkPhysicalDeviceFeatures physicalDeviceFeatures;


        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;

        // Create 1 Graphics Family Queue
        queueCreateInfo.queueFamilyIndex = familyIndices.graphicsFamily.value();
        queueCreateInfo.queueCount       = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;

        // Create Device create info for creating the logical device
        // with the required queue family
        VkDeviceCreateInfo deviceCreateInfo{};
        deviceCreateInfo.sType                = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceCreateInfo.pQueueCreateInfos    = &queueCreateInfo;
        deviceCreateInfo.queueCreateInfoCount = 1;

        deviceCreateInfo.pEnabledFeatures = &physicalDeviceFeatures;

        // Enable validation layer(@note: Deprecated)
        deviceCreateInfo.enabledExtensionCount = 0;

        auto validationLayers = std::vector{ "VK_LAYER_KHRONOS_validation" };

        if (_enabledValidationLayers)
        {
            deviceCreateInfo.enabledLayerCount   = static_cast<uint32_t>(validationLayers.size());
            deviceCreateInfo.ppEnabledLayerNames = validationLayers.data();
        }
        else
        {
            deviceCreateInfo.enabledLayerCount = 0;
        }

        // Instantiate Logical Device
        if (vkCreateDevice(_physicalDevice, &deviceCreateInfo, nullptr, &_vkDevice) != VK_SUCCESS)
            throw std::runtime_error("There was an error creating a vulkan logical device");

        // Queue are creating along with logical devices
        vkGetDeviceQueue(_vkDevice, familyIndices.graphicsFamily.value(), 0, &_graphicsQueue);
    }

    void pickPhysicalDevice()
    {
        uint32_t numDevices{};

        vkEnumeratePhysicalDevices(_vkInstance, &numDevices, nullptr);

        if (numDevices == 0)
            throw std::runtime_error("No devices with vulkan support present");

        std::vector<VkPhysicalDevice> physicalDevices(numDevices);
        vkEnumeratePhysicalDevices(_vkInstance, &numDevices, physicalDevices.data());

        std::multimap<int, VkPhysicalDevice> candidateDevices;

        // Associating each device with a score based on our requirements
        for (const auto& device : physicalDevices)
        {
            int score = rateDeviceSuitability(device);
            candidateDevices.insert(std::make_pair(score, device));
        }

        // Check if the candidate gpu supports our required features
        // Since we are using a reverse iterator, we will get the GPU with the largest score
        if (candidateDevices.rbegin()->first > 0)
        {
            _physicalDevice = candidateDevices.rbegin()->second;
        }
        else
        {
            throw std::runtime_error("Failed to find a suitable GPU!");
        }
    }

    int rateDeviceSuitability(VkPhysicalDevice device)
    {
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(device, &deviceProperties);

        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

        int score = 0;

        // Huge score increment to separate discrete from integrate GPU
        score += deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU ? 1000 : 0;

        // Max texture size
        score += deviceProperties.limits.maxImageDimension2D;

        // Don't support GPU that don't have a geometry shader
        if (!deviceFeatures.geometryShader)
            return 0;

        return score;
    }

    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device)
    {
        QueueFamilyIndices indices;

        uint32_t queueFamiliesCount;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamiliesCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamiliesCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamiliesCount, queueFamilyProperties.data());

        int i = 0;

        // Find at least one family that supports VK_QUEUE_GRAPHICS_BIT
        for (const auto& queueFamily : queueFamilyProperties)
        {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                indices.graphicsFamily = i;
            }
            ++i;

            if (indices.isComplete())
                break;
        }

        return indices;
    }

    /**
     * @brief Returns if a vulkan device(GPU) has certain feature set like being discrete or having geometry shaders.
     */
    [[maybe_unused]] bool isDeviceSuitable(VkPhysicalDevice device)
    {
        // Query basic features like name, type, vulkan version
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(device, &deviceProperties);

        // Query optional features like texture compression, 64-bit floats etc.
        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

        SDL_Log("Device Found!\n %s", deviceProperties.deviceName);

        // Only support Discrete GPU and ones that have a geometry shader
        // return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && deviceFeatures.geometryShader;
        QueueFamilyIndices indices = findQueueFamilies(device);

        return indices.isComplete();
    }




    VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                          const VkAllocationCallbacks* pAllocator,
                                          VkDebugUtilsMessengerEXT* pDebugMessenger)
    {
        auto func =
            (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr)
        {
            return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
        }
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }

    void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
                                       const VkAllocationCallbacks* pAllocator)
    {
        auto func =
            (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr)
            func(instance, debugMessenger, pAllocator);
    }

    bool queryLayerAvailability(const std::vector<const char*>& layers)
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
        SDL_Log("[Vulkan Validation]: %s", pCallbackData->pMessage);
        return VK_FALSE;
    }

private:
    SDL_Window* _window{ nullptr };
    static constexpr size_t WIDTH  = 1280;
    static constexpr size_t HEIGHT = 720;

    bool _gameRunning{ true };

    bool _enabledValidationLayers{ false };
    VkInstance _vkInstance{};
    VkDebugUtilsMessengerEXT _debugMessenger{};
    VkPhysicalDevice _physicalDevice{ VK_NULL_HANDLE };
    VkDevice _vkDevice{};
    VkQueue _graphicsQueue{};
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