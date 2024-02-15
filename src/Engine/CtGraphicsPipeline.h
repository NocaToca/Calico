#include <vulkan/vulkan.h>
#include <vector>
#include <string>
#include <cstdint>
#include <array>

class CtSwapchain;
class CtShader;
class CtSwapchain;
struct EngineSettings;
class CtDevice;

//So, I know I have been creating my own structs to basically take visual notes on how the API works, but I don't
//want to completely fill up this header file with all of that, so I will be creating them more directly (which does also mean it's more efficient!)
//If you're also learning more about Vulkan while reading this, don't worry! I still have notes, it's just a lot more in the cpp file
class CtGraphicsPipeline {

    public:
        static CtGraphicsPipeline* CreateGraphicsPipeline(EngineSettings settings, CtDevice* device, CtSwapchain* swapchain);

        static VkFormat FindSupportedFormat(VkPhysicalDevice* physical_device, const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
        static VkFormat FindDepthFormat(VkPhysicalDevice* physical_device);

    private:
        //Shaders
        std::vector<CtShader*> shaders;

        VkPipeline graphics_pipeline;
        VkRenderPass render_pass;
        VkDescriptorSetLayout descriptor_set_layout;
        VkPipelineLayout pipeline_layout;
        std::vector<VkDescriptorSet> descriptor_sets;

        //Pipeline creation
        void CreatePipeline(CtDevice* device, const std::vector<std::string>& shader_files, std::vector<uint32_t> stages, CtSwapchain* swapchain);

        void CreateVertexInputState(VkPipelineVertexInputStateCreateInfo& vertex_state_create_info, std::array<VkVertexInputAttributeDescription, 3> attribute_description,
                    VkVertexInputBindingDescription binding_description);
        VkPipelineInputAssemblyStateCreateInfo CreateInputAssemblyState();
        VkPipelineDynamicStateCreateInfo CreateDynamicState();
        VkPipelineViewportStateCreateInfo CreateViewportState(VkViewport& viewport, VkRect2D& scissor);
        VkPipelineRasterizationStateCreateInfo CreateRasterizerState();
        VkPipelineMultisampleStateCreateInfo CreateMultisampleState();
        VkPipelineColorBlendAttachmentState CreateBlendingAttachtmentState();
        VkPipelineColorBlendStateCreateInfo CreateBlendingState(VkPipelineColorBlendAttachmentState& color_blend_attachment);
        VkPipelineLayoutCreateInfo CreatePipelineLayout();
        VkPipelineDepthStencilStateCreateInfo CreateDepthStencilState();

        VkViewport CreateViewport(CtSwapchain* swapchain);
        VkRect2D CreateScissor(CtSwapchain* swapchain);

        //Descriptor creation
        void CreateDescriptorSetLayout(CtDevice* device);
        void CreateDescriptorSets();

        //Render pass creation
        void CreateRenderPass(CtDevice* device, CtSwapchain* swapchain);

        VkAttachmentDescription CreateColorAttachmentDescription(CtSwapchain* swapchain);
        VkAttachmentReference CreateColorAttachmentReference();
        VkSubpassDependency CreateSubpassDependency();
        VkAttachmentDescription CreateDepthAttachmentDescription(VkPhysicalDevice* physical_device);
        VkAttachmentReference CreateDepthAttachmentReference();
        VkSubpassDescription CreateSubpassDescription(VkAttachmentReference& color_attachment_reference, VkAttachmentReference& depth_attachment_reference);

        void CreateDescriptorSets(uint32_t max_frames_in_flight);

    friend class CtShader;
    friend class CtRenderer;
};