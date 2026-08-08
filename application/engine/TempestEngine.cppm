module;
#include <SDL3/sdl.h>
#include <string>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>
export module Engine; // TODO: Rename Later

namespace engine
{
    export class TempestEngine
    {
    public:
        TempestEngine() = default;
        void init();
        void run();
        void cleanup();

    private:
        void initVulkan();


        std::string appName{};
        std::string appVersion{};
        std::string appId{};

        SDL_Window* window{ nullptr };

        vk::raii::Context context{};
        vk::raii::Instance instance{ nullptr };

        std::string ENGINE_NAME{ "Tempest" };
    };

} // namespace engine