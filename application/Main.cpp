import Engine;

int main()
{
    engine::TempestEngine vulkanEngine{};
    vulkanEngine.init();
    vulkanEngine.run();
    vulkanEngine.cleanup();
}