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
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan.hpp>


export module TempestEngine:Vertex;


namespace tempest
{
    export struct Vertex
    {
        glm::vec3 pos;
        glm::vec3 color;
        glm::vec2 texCoord;

        [[nodiscard]] constexpr static vk::VertexInputBindingDescription getBindingDescription() noexcept
        {
            // Vertex binding description tells vulkan how to pass this data format to vulkan
            // when uploaded to GPU memory
            return { .binding = 0,
                     .stride  = sizeof(Vertex),
                     // Specifies whether to next data entry for each vertex or instance
                     .inputRate = vk::VertexInputRate::eVertex };
        }

        [[nodiscard]] constexpr static std::array<vk::VertexInputAttributeDescription, 3>
        getAttributeDescription() noexcept
        {
            // Describes a struct telling vulkan on how to extract a vertex attribute from a chunk of vertex data
            // from binding description
            return { {
                vk::VertexInputAttributeDescription{ .location = 0,
                                                     .binding  = 0,
                                                     .format   = vk::Format::eR32G32B32Sfloat,
                                                     .offset   = offsetof(Vertex, pos) }, // Position
                vk::VertexInputAttributeDescription{ .location = 1,
                                                     .binding  = 0,
                                                     .format   = vk::Format::eR32G32B32Sfloat,
                                                     .offset   = offsetof(Vertex, color) }, // Color
                vk::VertexInputAttributeDescription{ .location = 2,
                                                     .binding  = 0,
                                                     .format   = vk::Format::eR32G32Sfloat,
                                                     .offset   = offsetof(Vertex, texCoord) } // Texture Coordinates
            } };
        }

        [[nodiscard]] bool operator==(const Vertex& other) const
        { return pos == other.pos && color == other.color && texCoord == other.texCoord; }
    };
} // namespace tempest

export template <>
struct std::hash<tempest::Vertex>
{
    size_t operator()(const tempest::Vertex& vertex) const noexcept
    {
        return ((hash<glm::vec3>()(vertex.pos) ^ (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
            (hash<glm::vec2>()(vertex.texCoord) << 1);
    }
}; // namespace std