module;
/**
 * @file UniformBufferObject.cppm
 * @author Alan Abraham P Kochumon
 * @date Created on: September 09, 2026
 *
 * @brief Uniform buffer objects(UBO).
 *
 * @copyright Copyright (c) 2026 Alan Abraham P Kochumon
 */


#include <glm/glm.hpp>

export module TempestEngine:UBO;

namespace tempest
{
    export struct UniformBufferObject
    {
        glm::mat4 model, view, proj;
    };
} // namespace tempest