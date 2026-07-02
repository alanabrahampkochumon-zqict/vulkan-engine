#include <SDL3/SDL.h>
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

    void initVulkan() {}
    void mainLoop() {}
    void cleanUp() { SDL_DestroyWindow(_window); }

private:
    SDL_Window* _window{ nullptr };
    static constexpr size_t WIDTH  = 1280;
    static constexpr size_t HEIGHT = 720;
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