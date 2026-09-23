#pragma once
/**
 * @file RenderPipeline.h
 * @author Alan Abraham P Kochumon
 * @date Created on: September 23, 2026
 *
 * @brief Manages rendering pipeline.
 *
 * @copyright Copyright (c) 2026 Alan Abraham P Kochumon
 */

#include <string>

namespace tempest::renderer
{

    class RenderPipeline
    {
    public:
        RenderPipeline(const std::string& vertFilePath, const std::string& fragFilePath) noexcept;

    protected:
        void createGraphicsPipeline(const std::string& vertFilePath, const std::string& fragFilePath) noexcept;

    private:
        std::string _fragmentFilePath, _vertexFilePath;
    };
}