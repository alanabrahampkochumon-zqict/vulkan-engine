#include <string>

import TempestEngine;

int main()
{
    const std::string appName    = "Tempest Game Engine";
    const std::string appVersion = "0.0.1";
    const std::string appId      = "com.tempest.engine";

    engine::TempestEngine vulkanEngine{};
    vulkanEngine.init(appName, appVersion, appId, 1280, 720);
    vulkanEngine.run();
    vulkanEngine.cleanup();
}