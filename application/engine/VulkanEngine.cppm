export module Engine; // TODO: Rename Later
import std;

namespace engine
{
    export class VulkanEngine
    {
    public:
        constexpr void init();
        constexpr void run();
        constexpr void cleanup();
    };

} // namespace engine