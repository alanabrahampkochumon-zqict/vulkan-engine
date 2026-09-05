module;
/**
 * @file Vertex.cppm
 * @author Alan Abraham P Kochumon
 * @date Created on: September 05, 2026
 *
 * @brief Structure for shader vertex.
 *
 * @copyright Copyright (c) 2026 Alan Abraham P Kochumon
 */

#include <array>
#include <glm/glm.hpp>

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan.hpp>
export module TempestEngine:Vertex;


namespace tempest
{
    export struct Vertex
    {
        glm::vec2 pos;
        glm::vec3 color;

        [[nodiscard]] constexpr static vk::VertexInputBindingDescription getBindingDescription() noexcept
        {
            // Vertex binding description tells vulkan how to pass this data format to vulkan
            // when uploaded to GPU memory
            return { .binding = 0,
                     .stride  = sizeof(Vertex),
                     // Specifies whether to next data entry for each vertex or instance
                     .inputRate = vk::VertexInputRate::eVertex };
        }

        [[nodiscard]] constexpr static std::array<vk::VertexInputAttributeDescription, 2>
        getAttributeDescription() noexcept
        {
            // Describes a struct telling vulkan on how to extract a vertex attribute from a chunk of vertex data
            // from binding description
            return std::array{
                vk::VertexInputAttributeDescription{ .location = 0,
                                                     .binding  = 0,
                                                     .format   = vk::Format::eR32G32Sfloat,
                                                     .offset   = offsetof(Vertex, pos) }, // Position
                vk::VertexInputAttributeDescription{ .location = 1,
                                                     .binding  = 0,
                                                     .format   = vk::Format::eR32G32B32Sfloat,
                                                     .offset   = offsetof(Vertex, color) } // Color
            };
        }
    };
} // namespace tempest