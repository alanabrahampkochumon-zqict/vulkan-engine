#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <cstdint>
#include <cstdlib>
#include <exception>
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

    void initVulkan() { createInstance(); }

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

    void cleanUp() { SDL_DestroyWindow(_window); }

    void createInstance()
    {
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

        createInfo.enabledExtensionCount   = instanceExtensionCount;
        createInfo.ppEnabledExtensionNames = instanceExtensions;

        createInfo.enabledExtensionCount = 0;

        // Create the vulkan instance
        VkResult result = vkCreateInstance(&createInfo, nullptr, &_vkInstance);

        if (result == VK_SUCCESS)
            SDL_Log("Initialized Vulkan!");
        else
            SDL_Log("There was an error creating the Vulkan instance.");
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