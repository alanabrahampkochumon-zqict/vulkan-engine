export module Engine; // TODO: Rename Later

namespace engine
{
    export class VulkanEngine
    {
    public:
        void init();
        void run();
        void cleanup();
    };

} // namespace engine