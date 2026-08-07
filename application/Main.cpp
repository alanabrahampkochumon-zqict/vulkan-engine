#include <iostream>
import Engine;

int main()
{
    engine::VulkanEngine vulkanEngine{};
    vulkanEngine.init();
    vulkanEngine.run();
    vulkanEngine.cleanup();
}