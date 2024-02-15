#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>
#include <vector>
#include <array>


struct CtVertex {
    glm::vec2 position;
    glm::vec3 color;
    glm::vec2 texCoord;

    static VkVertexInputBindingDescription GetBindingDescription(){
        VkVertexInputBindingDescription binding_description{};

        binding_description.binding = 0;
        binding_description.stride = sizeof(CtVertex);
        binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return binding_description;
    }

    static std::array<VkVertexInputAttributeDescription, 3> GetAttributeDescriptions(){
        std::array<VkVertexInputAttributeDescription, 3> attribute_descriptions{};

        attribute_descriptions[0].binding = 0;
        attribute_descriptions[0].location = 0;
        attribute_descriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
        attribute_descriptions[0].offset = offsetof(CtVertex, position);

        attribute_descriptions[1].binding = 0;
        attribute_descriptions[1].location = 1;
        attribute_descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attribute_descriptions[1].offset = offsetof(CtVertex, color);

        attribute_descriptions[2].binding = 0;
        attribute_descriptions[2].location = 2;
        attribute_descriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
        attribute_descriptions[2].offset = offsetof(CtVertex, texCoord);

        return attribute_descriptions;
    }

    bool operator==(const CtVertex& other) const{
        return position == other.position && color == other.color && texCoord == other.texCoord;
    }
};

const std::vector<CtVertex> test_vertices = {
    {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
    {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
    {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
    {{-0.5f, 0.5f}, {1.0f, 1.0f, 0.0f}, {0.0f, 0.0f}}
};

const std::vector<uint16_t> test_indices = {
    0, 1, 2, 2, 3, 0
};