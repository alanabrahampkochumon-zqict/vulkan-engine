/**
 * @file RenderPipeline.cpp
 * @author Alan Abraham P Kochumon
 * @date Created on: September 23, 2026
 *
 * @brief Implementation of functions declared in RenderPipeline.h
 *
 * @copyright Copyright (c) 2026 Alan Abraham P Kochumon
 */

#include "RenderPipeline.h"

#include "../utils/FileReader.h"

#include <iostream>


namespace tempest::renderer
{
    using namespace tempest::utils;

    RenderPipeline::RenderPipeline(const std::string& vertFilePath, const std::string& fragFilePath) noexcept
    { createGraphicsPipeline(vertFilePath, fragFilePath); }


    void RenderPipeline::createGraphicsPipeline(const std::string& vertFilePath,
                                                const std::string& fragFilePath) noexcept
    {
        const auto vertFile = readFile(vertFilePath);
        const auto fragFile = readFile(fragFilePath);

        std::cout << "Vertex: " << vertFile.size() << '\n';
        std::cout << "Fragment: " << fragFile.size() << '\n';
    }


} // namespace tempest::renderer
