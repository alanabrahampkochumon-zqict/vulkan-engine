#include <SDL3/SDL.h>
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
        initVulkan();
        mainLoop();
        cleanUp();
    }

private:
    void initVulkan() {}
    void mainLoop() {}
    void cleanUp() {}
};

int main()
{
    // Set the application metadata
    // SDL_SetAppMetadata(APP_NAME, APP_VERSION, APP_ID);
    // SDL_Log("Hello, world!");
    // if (!SDL_Init(SDL_INIT_VIDEO))
    //     SDL_Log("There was an error initializing SDL");
    // SDL_Window* window = SDL_CreateWindow(APP_NAME, 1280, 720, 0);
    //
    //
    // // Cleanup
    // SDL_DestroyWindow(window);

    HelloTriangleApplication app;

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