#include <string>
#include <print>
#include <iostream>

import TempestEngine;

int main()
{
    const std::string appName    = "Tempest Game Engine";
    const std::string appVersion = "0.0.1";
    const std::string appId      = "com.tempest.engine";

    try{
        engine::TempestEngine vulkanEngine{};
        vulkanEngine.init(appName, appVersion, appId, 1280, 720);
        vulkanEngine.run();
        vulkanEngine.cleanup();
    } catch (const std::exception& e)
    {
        std::println(std::cerr, "Engine Exception\n{}", e.what());
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}