module;
#include <SDL3/sdl.h>
#include <string>

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

export module TempestEngine; // TODO: Rename Later

namespace engine
{
    export class TempestEngine
    {
    public:
        TempestEngine() = default;
        void init(const std::string& applicationName, const std::string& version, const std::string& id,
                  size_t width, size_t height);
        void run();
        void cleanup();

    private:
        void initVulkan();


        std::string appName{}, appVersion{}, appId{};

        size_t width{}, height{};


        SDL_Window* window{ nullptr };

        vk::raii::Context context{};
        vk::raii::Instance instance{ nullptr };

        std::string ENGINE_NAME{ "Tempest" };
    };

} // namespace engine