#include <SDL3/SDL.h>

static auto APP_NAME    = "Vulkan Renderer";
static auto APP_ID      = "com.alan.vulkan_renderer";
static auto APP_VERSION = "1.0.0";

int main()
{
    // Set the application metadata
    SDL_SetAppMetadata(APP_NAME, APP_VERSION, APP_ID);
    SDL_Log("Hello, world!");
    if (!SDL_Init(SDL_INIT_VIDEO))
        SDL_Log("There was an error initializing SDL");
    SDL_Window* window = SDL_CreateWindow(APP_NAME, 1280, 720, 0);
}