#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <map>
#include <optional>
#include <print>
#include <set>
#include <vector>
#include <vulkan/vulkan.h>


static auto APP_NAME    = "Vulkan Renderer";
static auto APP_ID      = "com.alan.vulkan_renderer";
static auto APP_VERSION = "1.0.0";

struct QueueFamilyIndices
{
    std::optional<uint32_t> graphicsFamily{ std::nullopt };
    std::optional<uint32_t> presentFamily{ std::nullopt }; // Queue for presenting image to surface

    /**
     * @brief Returns whether the current queue family has a device feature set.
     */
    bool isComplete() { return graphicsFamily.has_value() && presentFamily.has_value(); }
};


struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities;      /// Min/Max number, width, height of swap chain images.
    std::vector<VkSurfaceFormatKHR> format;     /// Pixel format, color space etc.
    std::vector<VkPresentModeKHR> presentModes; /// Presentation Mode
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

        _window = SDL_CreateWindow(APP_NAME, WIDTH, HEIGHT, SDL_WINDOW_VULKAN);
    }

    void initVulkan()
    {
        createInstance();
        queryAvailableExtensions();
        setupDebugMessenger();
        createSurface();
        pickPhysicalDevice();
        createLogicalDevice();
        createSwapChain();
        createImageViews();
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
        for (const auto& swapChainImageView : _swapChainImageViews)
            vkDestroyImageView(_vkDevice, swapChainImageView, nullptr);

        vkDestroySwapchainKHR(_vkDevice, _vkSwapChain, nullptr);
        vkDestroySurfaceKHR(_vkInstance, _vkSurface, nullptr);
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
        constexpr bool enableValidation = true;
        _enabledValidationLayers        = true;
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


    void createImageViews()
    {
        // Resize the imageviews container to be the size of retrieved images
        _swapChainImageViews.resize(_swapChainImages.size());

        // Loop through each images and create an imageview for each
        for (std::size_t i = 0; i < _swapChainImages.size(); ++i)
        {
            VkImageViewCreateInfo imageViewCreateInfo{};
            imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            imageViewCreateInfo.image = _swapChainImages[i];

            imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D; // Type of image, 1D, 2D, 3D, Cube map
            imageViewCreateInfo.format   = _swapChainFormat;

            // Apply no color channel swizzling
            imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

            // Specify images purpose and which part of image should be accessed
            imageViewCreateInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
            imageViewCreateInfo.subresourceRange.baseMipLevel   = 0;
            imageViewCreateInfo.subresourceRange.levelCount     = 1;
            imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
            imageViewCreateInfo.subresourceRange.layerCount     = 1;

            // Create the image view
            if (vkCreateImageView(_vkDevice, &imageViewCreateInfo, nullptr, &_swapChainImageViews[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("There was an error creating the swapchain ImageView!");
            }
        }
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

    void createSurface()
    {
        if (!SDL_Vulkan_CreateSurface(_window, _vkInstance, nullptr, &_vkSurface) != VK_SUCCESS)
            throw std::runtime_error("There was an error creating a rendering surface!");
    }

    void createLogicalDevice()
    {
        QueueFamilyIndices familyIndices = findQueueFamilies(_physicalDevice);
        float queuePriority              = 1.0f; // Must specify a priority even if it's the one queue

        // Used Device features
        VkPhysicalDeviceFeatures physicalDeviceFeatures{}; // NOTE: Always empty initialize else the prog will crash

        std::set<uint32_t> queueFamilies = { familyIndices.graphicsFamily.value(),
                                             familyIndices.presentFamily.value() };
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        for (auto& queueFamily : queueFamilies)
        {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            // Create a single Queue for each type of queue we want
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount       = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;

            queueCreateInfos.push_back(queueCreateInfo);
        }

        // Create Device create info for creating the logical device
        // with the required queue family
        VkDeviceCreateInfo deviceCreateInfo{};
        deviceCreateInfo.sType                = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceCreateInfo.pQueueCreateInfos    = queueCreateInfos.data();
        deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        deviceCreateInfo.pEnabledFeatures     = &physicalDeviceFeatures;
        deviceCreateInfo.enabledLayerCount    = 0;

        // Enable Swap chain and other layers
        deviceCreateInfo.enabledExtensionCount   = static_cast<uint32_t>(deviceExtensions.size());
        deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();

        // Instantiate Logical Device
        if (vkCreateDevice(_physicalDevice, &deviceCreateInfo, nullptr, &_vkDevice) != VK_SUCCESS)
            throw std::runtime_error("There was an error creating a vulkan logical device");

        // Queue are creating along with logical devices
        vkGetDeviceQueue(_vkDevice, familyIndices.graphicsFamily.value(), 0, &_graphicsQueue);
        vkGetDeviceQueue(_vkDevice, familyIndices.presentFamily.value(), 0, &_presentQueue);
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
            // Use the first suitable device
            if (isDeviceSuitable(device))
            {
                _physicalDevice = device;
                break;
            }
            // int score = rateDeviceSuitability(device);
            // candidateDevices.insert(std::make_pair(score, device));
        }

        // Check if the candidate gpu supports our required features
        // Since we are using a reverse iterator, we will get the GPU with the largest score
        // if (candidateDevices.rbegin()->first > 0)
        // {
        //     _physicalDevice = candidateDevices.rbegin()->second;
        // }
        // else
        // {
        //     throw std::runtime_error("Failed to find a suitable GPU!");
        // }
    }

    [[maybe_unused]] int rateDeviceSuitability(VkPhysicalDevice device)
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

    QueueFamilyIndices findQueueFamilies(const VkPhysicalDevice device) const
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
                break;
            }
            ++i;

            // if (indices.isComplete())
            //     break;
        }

        /////////////////////////////////
        /// FINDING PRESENTING SUPPORT //
        /////////////////////////////////
        VkBool32 presentFamily;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, _vkSurface, &presentFamily);
        if (presentFamily)
            indices.presentFamily = i; // Might return the same queue(one queue with both graphics and present support)

        return indices;
    }

    void createSwapChain()
    {
        // Query the swap chain support details
        SwapChainSupportDetails supportDetails = querySwapChainSupportDetails(_physicalDevice);

        // Choose an apt format, present mode and extent(resolution)
        VkSurfaceFormatKHR format    = chooseSurfaceFormat(supportDetails.format);
        VkPresentModeKHR presentMode = choosePresentMode(supportDetails.presentModes);
        VkExtent2D extent            = chooseSwapExtent(supportDetails.capabilities);

        // At least increment image count by 1 to ensure we don't to wait
        // for driver to complete its internal operation to get another image.
        uint32_t imageCount = supportDetails.capabilities.minImageCount + 1;

        // If we however cant have more than the min, clamp it to the max images supported.
        // Zero => No maximum
        if (supportDetails.capabilities.minImageCount > 0 && imageCount > supportDetails.capabilities.maxImageCount)
            imageCount = supportDetails.capabilities.maxImageCount;


        // Create swap chain create info
        VkSwapchainCreateInfoKHR swapChainCreateInfo{};
        swapChainCreateInfo.sType   = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        swapChainCreateInfo.surface = _vkSurface;

        // Setup the colorspace and image format
        swapChainCreateInfo.minImageCount   = imageCount;
        swapChainCreateInfo.imageFormat     = format.format;
        swapChainCreateInfo.imageColorSpace = format.colorSpace;

        swapChainCreateInfo.imageArrayLayers = 1; // Doesn't need more than 1 unless for stereoscopic 3D
        // To render to separate image use `VK_IMAGE_USAGE_TRANSFER_DST_BIT`
        swapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; // Use the image for direct rendering

        // Setup queue sharing mode
        QueueFamilyIndices indices = findQueueFamilies(_physicalDevice);


        uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

        // If Graphics family and present families are separate queue, we need to
        // set image sharing mode to concurrent
        if (indices.graphicsFamily != indices.presentFamily)
        {
            // Shareability without explicit ownership requirements
            swapChainCreateInfo.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
            swapChainCreateInfo.queueFamilyIndexCount = 2;
            swapChainCreateInfo.pQueueFamilyIndices   = queueFamilyIndices;
        }
        else
        {
            // Set the preset mode to Exclusive
            // Ownership is exclusive and implicit transfer of ownership is required
            swapChainCreateInfo.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
            swapChainCreateInfo.queueFamilyIndexCount = 0;       // Optional
            swapChainCreateInfo.pQueueFamilyIndices   = nullptr; // Optional
        }

        // Apply no transformation to the swap chain image
        swapChainCreateInfo.preTransform = supportDetails.capabilities.currentTransform;

        // Setup the present mode
        swapChainCreateInfo.presentMode = presentMode;
        swapChainCreateInfo.clipped     = VK_TRUE; // Clip any invisible areas

        // Setup composite transparency (window transparency)
        swapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; // No transparency


        // Setup the extent
        swapChainCreateInfo.imageExtent = extent;

        // Pass in the old swap chain
        swapChainCreateInfo.oldSwapchain = VK_NULL_HANDLE;


        // Create the swap chain
        if (vkCreateSwapchainKHR(_vkDevice, &swapChainCreateInfo, nullptr, &_vkSwapChain) != VK_SUCCESS)
        {
            throw std::runtime_error("There was an error creating a swapchain");
        }

        // Retrieve the swap chain images
        uint32_t numImages;
        vkGetSwapchainImagesKHR(_vkDevice, _vkSwapChain, &numImages, nullptr);
        _swapChainImages.resize(numImages);
        vkGetSwapchainImagesKHR(_vkDevice, _vkSwapChain, &numImages, _swapChainImages.data());

        // Store SwapChain format and extent for future use
        _swapChainExtend = extent;
        _swapChainFormat = format.format;
    }

    VkSurfaceFormatKHR chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
    {

        // If we support an srgb 32-bit color format
        // choose it
        for (auto& format : availableFormats)
        {
            if (format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR && format.format == VK_FORMAT_B8G8R8A8_SRGB)
                return format;
        }

        // If not choose the first color format supported
        return availableFormats[0];
    }

    VkPresentModeKHR choosePresentMode(const std::vector<VkPresentModeKHR>& availableModes)
    {
        // Choose the mailbox present mode if possible for minimal latency
        for (auto& mode : availableModes)
        {
            if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
                return mode;
        }

        // IF not supported fall back to FIFO which is guaranteed to be supported
        return VK_PRESENT_MODE_FIFO_KHR;
    }


    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
    {
        // Resolution
        // If the swap chain has no maximum extent return it(essentially equal to uint32_t's max value
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
        {
            return capabilities.currentExtent;
        }


        // Else get the window height and width and clamp it between the swap chain's min and max extent.

        int width, height;
        SDL_GetWindowSizeInPixels(_window, &width, &height);

        VkExtent2D actualExtent = { .width = static_cast<uint32_t>(width), .height = static_cast<uint32_t>(height) };

        actualExtent.width =
            std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height =
            std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }


    /**
     * @brief Returns if a vulkan device(GPU) has certain feature set like being discrete or having geometry shaders.
     */
    bool isDeviceSuitable(VkPhysicalDevice device)
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

        const bool isExtensionSupported = checkDeviceExtensionSupport(device);

        bool isSwapChainSuitable{ false };
        if (isExtensionSupported)
        {
            auto swapChainDetails = querySwapChainSupportDetails(device);
            isSwapChainSuitable   = !swapChainDetails.presentModes.empty() && !swapChainDetails.format.empty();
        }

        return indices.isComplete() && isExtensionSupported && isSwapChainSuitable;
    }


    bool checkDeviceExtensionSupport(VkPhysicalDevice device)
    {
        // Query the available extensions
        uint32_t numExtensions;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &numExtensions, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(numExtensions);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &numExtensions, availableExtensions.data());

        // Form a set to store our required extensions
        std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

        // Every time our required extension is in the devices supported extension list
        // remove it
        for (const auto& [extensionName, specVersion] : availableExtensions)
        {
            requiredExtensions.erase(extensionName);
        }

        // Return true if all our mandatory extensions are supported
        return requiredExtensions.empty();
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

    SwapChainSupportDetails querySwapChainSupportDetails(VkPhysicalDevice device)
    {

        SwapChainSupportDetails details;

        // Query Device capabilities
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, _vkSurface, &details.capabilities);


        // Query format count and supported surface formats
        uint32_t numFormats;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, _vkSurface, &numFormats, nullptr);

        if (numFormats != 0)
        {
            details.format.resize(numFormats);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, _vkSurface, &numFormats, details.format.data());
        }

        // Query present modes
        uint32_t numPresentModes;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, _vkSurface, &numPresentModes, nullptr);

        if (numPresentModes != 0)
        {
            details.presentModes.resize(numPresentModes);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, _vkSurface, &numPresentModes,
                                                      details.presentModes.data());
        }
        return details;
    }

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                        VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                        void* pUserData)
    {
        if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Vulkan Validation]: (ERROR) %s", pCallbackData->pMessage);
        else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "[Vulkan Validation]: (WARNING) %s", pCallbackData->pMessage);
        else
            SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "[Vulkan Validation]: %s", pCallbackData->pMessage);
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
    VkQueue _graphicsQueue{}, _presentQueue{};
    VkSurfaceKHR _vkSurface{};
    VkSwapchainKHR _vkSwapChain{};
    VkFormat _swapChainFormat{};
    VkExtent2D _swapChainExtend{};
    std::vector<VkImage> _swapChainImages{};
    std::vector<VkImageView> _swapChainImageViews{};

    // Swapchain support
    std::vector<const char*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME // "VK_KHR_swapchain
    };
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