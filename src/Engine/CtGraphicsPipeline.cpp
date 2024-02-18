#include "CtGraphicsPipeline.h"
#include "CtVertex.h"
#include "CtSwapchain.h"
#include "CtShader.h"
#include "CtDevice.h"
#include "Engine.h"
#include <array>
#include <stdexcept>

CtGraphicsPipeline* CtGraphicsPipeline::CreateGraphicsPipeline(EngineSettings settings, CtDevice* device, CtSwapchain* swapchain){

    CtGraphicsPipeline* ct_graphics_pipeline = new CtGraphicsPipeline();

    ct_graphics_pipeline->CreateDescriptorSetLayout(device);

    printf("Created Descriptor Set Layouts.\n");

    ct_graphics_pipeline->CreateRenderPass(device, swapchain);

    printf("Created Render Pass.\n");

    ct_graphics_pipeline->CreatePipeline(device, settings.graphics_settings.shader_files, settings.graphics_settings.shader_stages, swapchain);

    printf("Created Graphics Pipeline.\n");

    return ct_graphics_pipeline;
}

void CtGraphicsPipeline::CreatePipeline(CtDevice* device, const std::vector<std::string>& shader_files, std::vector<uint32_t> stages, CtSwapchain* swapchain){
    VkGraphicsPipelineCreateInfo pipeline_info {};    

    pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

    //We have to do some work with creating our shaders.
    int i = 0;
    for(const auto& shader_file_name : shader_files){
        shaders.push_back(CtShader::CreateShader(device, shader_file_name, (CtShaderPipelineStage)stages[i]));
        i++;
    }
    
    std::vector<VkPipelineShaderStageCreateInfo> shader_stages;
    // shader_stages.resize(i);

    for(uint32_t j = 0; j < i; j++){
        // printf("Iteration: %d.\n", i);
        VkPipelineShaderStageCreateInfo vk_shader_create_info {};

        shaders[j]->CreateShaderPipelineInfo(vk_shader_create_info);
        shader_stages.push_back(vk_shader_create_info);
    }

    pipeline_info.stageCount = static_cast<uint32_t>(shader_stages.size()); //should be i but I think this makes more intuitive since sight-reading 
    pipeline_info.pStages = shader_stages.data();

    //Now we can move onto the easier stuff!
    //Vertex
    auto binding_description = CtVertex::GetBindingDescription();
    auto attribute_description = CtVertex::GetAttributeDescriptions(); 

    VkPipelineVertexInputStateCreateInfo vertex_info {};
    CreateVertexInputState(vertex_info, attribute_description, binding_description);
    pipeline_info.pVertexInputState = &vertex_info;

    //Input assembly
    VkPipelineInputAssemblyStateCreateInfo input_assembly = CreateInputAssemblyState();
    pipeline_info.pInputAssemblyState = &input_assembly;

    //Viewport info
    VkViewport viewport = CreateViewport(swapchain);
    VkRect2D scissor = CreateScissor(swapchain);

    VkPipelineViewportStateCreateInfo viewport_info = CreateViewportState(viewport, scissor);
    pipeline_info.pViewportState = &viewport_info;

    //Rasterization info
    VkPipelineRasterizationStateCreateInfo rasterization_info = CreateRasterizerState();
    pipeline_info.pRasterizationState = &rasterization_info;

    //Multisampling info
    VkPipelineMultisampleStateCreateInfo multisampling_info = CreateMultisampleState();
    pipeline_info.pMultisampleState = &multisampling_info;

    //Color blending
    VkPipelineColorBlendAttachmentState color_attachment = CreateBlendingAttachtmentState();
    VkPipelineColorBlendStateCreateInfo color_blend_info = CreateBlendingState(color_attachment);
    pipeline_info.pColorBlendState = &color_blend_info;

    //Depth Stencil
    VkPipelineDepthStencilStateCreateInfo depth_stencil_info = CreateDepthStencilState();
    pipeline_info.pDepthStencilState = &depth_stencil_info;

    //Dynamic State
    VkPipelineDynamicStateCreateInfo dynamic_state_info = CreateDynamicState();
    pipeline_info.pDynamicState = &dynamic_state_info;

    //Pipeline layout!
    VkPipelineLayoutCreateInfo pipeline_layout_info = CreatePipelineLayout();

    VkResult result = vkCreatePipelineLayout(*(device->GetInterfaceDevice()), &pipeline_layout_info, nullptr, &pipeline_layout);

    switch(result){
        case VK_SUCCESS:
            break;
        case VK_ERROR_OUT_OF_HOST_MEMORY:
            throw std::runtime_error("Failed to create a pipeline layout. Out of host memory.\n");
        case VK_ERROR_OUT_OF_DEVICE_MEMORY:
            throw std::runtime_error("Failed to create a pipeline layout. Out of device memory.\n");
        default:
            throw std::runtime_error("Failed to create a pipeline layout!");
    }
    

    pipeline_info.layout = pipeline_layout;

    //Now some misc stuff
    pipeline_info.renderPass = render_pass;
    pipeline_info.subpass = 0;

    result = vkCreateGraphicsPipelines(*(device->GetInterfaceDevice()), VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &graphics_pipeline);
    switch(result){
        case VK_SUCCESS:
            break;
        case VK_PIPELINE_COMPILE_REQUIRED_EXT:
            break;
        case VK_ERROR_OUT_OF_HOST_MEMORY:
            throw std::runtime_error("Failed to create graphics pipeline. Out of host memory.\n");
        case VK_ERROR_OUT_OF_DEVICE_MEMORY:
            throw std::runtime_error("Failed to create graphics pipeline. Out of device memory.\n");
        case VK_ERROR_INVALID_SHADER_NV:
            throw std::runtime_error("Failed to create graphics pipeline. Shader error.\n");
        default:
            throw std::runtime_error("Failed to create graphics pipeline.\n");
    }

    //Now we just have to tell our shaders to destroy their modules
    for(const auto& shader : shaders){
        shader->DestroyShaderModule(device->GetInterfaceDevice());
    }
}

void CtGraphicsPipeline::CreateRenderPass(CtDevice* device, CtSwapchain* swapchain){
    VkRenderPassCreateInfo render_pass_info {};

    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;

    //Lets grab the dependency
    VkSubpassDependency dependency = CreateSubpassDependency();
    render_pass_info.dependencyCount = 1;
    render_pass_info.pDependencies = &dependency;

    //Now let's create the subpass
    VkAttachmentReference color_attachment_reference = CreateColorAttachmentReference();
    VkAttachmentReference depth_attachment_reference = CreateDepthAttachmentReference();
    VkSubpassDescription subpass = CreateSubpassDescription(color_attachment_reference, depth_attachment_reference);

    //Finally, let's create the descriptions
    VkAttachmentDescription color_attachment_description = CreateColorAttachmentDescription(swapchain);
    VkAttachmentDescription depth_attachment_description = CreateDepthAttachmentDescription(device->GetPhysicalDevice());

    //Make sure these indices are inline with what we put for create color attachment reference and depth
    std::array<VkAttachmentDescription, 2> attachments = {color_attachment_description, depth_attachment_description};

    render_pass_info.attachmentCount = static_cast<uint32_t>(attachments.size());
    render_pass_info.pAttachments = attachments.data();
    render_pass_info.subpassCount = 1;
    render_pass_info.pSubpasses = &subpass;


    if(vkCreateRenderPass(*(device->GetInterfaceDevice()), &render_pass_info, nullptr, &render_pass) != VK_SUCCESS){
        throw std::runtime_error("Failed to create render pass!");
    }
}


//Comparitively, this is pretty small. So we'll just keep everything in a function to avoid making too many
void CtGraphicsPipeline::CreateDescriptorSetLayout(CtDevice* device){
    VkDescriptorSetLayoutBinding ubo_layout_binding{};

    ubo_layout_binding.binding = 0;
    ubo_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    ubo_layout_binding.descriptorCount = 1;

    ubo_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    ubo_layout_binding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding sampler_layout_binding{};
    sampler_layout_binding.binding = 1;
    sampler_layout_binding.descriptorCount = 1;
    sampler_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    sampler_layout_binding.pImmutableSamplers = nullptr;
    sampler_layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    std::array<VkDescriptorSetLayoutBinding, 2> bindings = {ubo_layout_binding, sampler_layout_binding};

    VkDescriptorSetLayoutCreateInfo layout_info{};
    layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layout_info.bindingCount = static_cast<uint32_t>(bindings.size());
    layout_info.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(*(device->GetInterfaceDevice()), &layout_info, nullptr, &descriptor_set_layout) != VK_SUCCESS){
        throw std::runtime_error("Failed to create a descriptor set layout");
    }
}

void CtGraphicsPipeline::CreateVertexInputState(VkPipelineVertexInputStateCreateInfo& vertex_state_create_info, 
    std::array<VkVertexInputAttributeDescription, 3> attribute_description,
    VkVertexInputBindingDescription binding_description){

    //Obviously, we have to define the structure type first
    vertex_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    //Next we have to get our binding descriptions
    //We use our CtVertex class defined in CtVertex.h to represent vertices. We have a helpful function to
    //Grab our binding descriptions!

    //We only ever have one for now - multiple would be for multiple different representation classes
    vertex_state_create_info.vertexBindingDescriptionCount = 1;
    vertex_state_create_info.pVertexBindingDescriptions = &binding_description;

    //Now we do the same thing for our attribute descriptions

    //And then we fill and return!
    vertex_state_create_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(attribute_description.size());
    vertex_state_create_info.pVertexAttributeDescriptions = attribute_description.data();
}

VkPipelineInputAssemblyStateCreateInfo CtGraphicsPipeline::CreateInputAssemblyState(){
    VkPipelineInputAssemblyStateCreateInfo input_assembly_state_create_info {};

    //Fill out the structure type
    input_assembly_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;

    //Let's set to basic input topology
    input_assembly_state_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    //This controls whether a special vertex index value is treated as restarting the assembly of primitives, usually the last index value
    input_assembly_state_create_info.primitiveRestartEnable = VK_FALSE;

    return input_assembly_state_create_info;
}

VkPipelineViewportStateCreateInfo CtGraphicsPipeline::CreateViewportState(VkViewport& viewport, VkRect2D& scissor){
    VkPipelineViewportStateCreateInfo viewport_state_create_info {};

    //Fill out structure type
    viewport_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;

    //Fill out viewport information
    viewport_state_create_info.viewportCount = 1;
    viewport_state_create_info.pViewports = &viewport;

    //Fill out scissor information
    viewport_state_create_info.scissorCount = 1;
    viewport_state_create_info.pScissors = &scissor;

    return viewport_state_create_info;
}

VkPipelineRasterizationStateCreateInfo CtGraphicsPipeline::CreateRasterizerState(){

    VkPipelineRasterizationStateCreateInfo rasterizer{};

    //Fill out structure type
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;

    //We dont want depth clamp for now
    rasterizer.depthClampEnable = VK_FALSE;

    //We don't want to discard the rasterizer ever at the moment
    rasterizer.rasterizerDiscardEnable = VK_FALSE;

    //We just want to fill in vertices
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;

    //We are in 2D
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

    //Depth stuff
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f;
    rasterizer.depthBiasClamp = 0.0f;
    rasterizer.depthBiasSlopeFactor = 0.0f;
    return rasterizer;
}

VkPipelineMultisampleStateCreateInfo CtGraphicsPipeline::CreateMultisampleState(){
    VkPipelineMultisampleStateCreateInfo multisampling{};
    //Structure
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;

    //We don't really need to use multi-sampling at the moment, so I'm just going to leave it at these settings!
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f; 
    multisampling.pSampleMask = nullptr; 
    multisampling.alphaToCoverageEnable = VK_FALSE; 
    multisampling.alphaToOneEnable = VK_FALSE; 

    return multisampling;
}

VkPipelineColorBlendAttachmentState CtGraphicsPipeline::CreateBlendingAttachtmentState(){
    VkPipelineColorBlendAttachmentState color_blend_attachment{};

    color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    color_blend_attachment.blendEnable = VK_FALSE;
    color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
    color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;

    return color_blend_attachment;
}
VkPipelineColorBlendStateCreateInfo CtGraphicsPipeline::CreateBlendingState(VkPipelineColorBlendAttachmentState& color_blend_attachment){
    VkPipelineColorBlendStateCreateInfo color_blending{};

    color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blending.logicOpEnable = VK_FALSE;
    color_blending.logicOp = VK_LOGIC_OP_COPY;
    color_blending.attachmentCount = 1;
    color_blending.pAttachments = &color_blend_attachment;

    return color_blending;
}
VkPipelineLayoutCreateInfo CtGraphicsPipeline::CreatePipelineLayout(){
    VkPipelineLayoutCreateInfo pipeline_layout_info{};

    pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_info.setLayoutCount = 1;
    pipeline_layout_info.pSetLayouts = &descriptor_set_layout;
    pipeline_layout_info.pushConstantRangeCount = 0;
    pipeline_layout_info.pPushConstantRanges = nullptr;

    return pipeline_layout_info;
}
VkPipelineDepthStencilStateCreateInfo CtGraphicsPipeline::CreateDepthStencilState(){
    VkPipelineDepthStencilStateCreateInfo depth_stencil{};

    depth_stencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depth_stencil.depthTestEnable = VK_TRUE;
    depth_stencil.depthWriteEnable = VK_TRUE;

    depth_stencil.depthCompareOp = VK_COMPARE_OP_LESS;

    depth_stencil.depthBoundsTestEnable = VK_FALSE;
    depth_stencil.minDepthBounds = 0.0f;
    depth_stencil.maxDepthBounds = 1.0f;

    depth_stencil.stencilTestEnable = VK_FALSE;
    depth_stencil.front = {};
    depth_stencil.back = {};

    return depth_stencil;
}

VkViewport CtGraphicsPipeline::CreateViewport(CtSwapchain* swapchain){
    //Let's create our viewport
    VkViewport viewport {};

    //This is pretty simple!
    viewport.x = 0.0f;
    viewport.y = 0.0f;

    viewport.width = (float)(swapchain->GetSwapchainExtent().width);
    viewport.height = (float)(swapchain->GetSwapchainExtent().height);

    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    return viewport;
}

VkRect2D CtGraphicsPipeline::CreateScissor(CtSwapchain* swapchain){
    VkRect2D scissor {};
    scissor.offset = {0, 0};
    scissor.extent = swapchain->GetSwapchainExtent();

    return scissor;
}

VkAttachmentDescription CtGraphicsPipeline::CreateColorAttachmentDescription(CtSwapchain* swapchain){
    VkAttachmentDescription color_attachment{};

    color_attachment.format = swapchain->swapchain_image_format;
    color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp =VK_ATTACHMENT_STORE_OP_STORE;

    color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    return color_attachment;
}

VkAttachmentReference CtGraphicsPipeline::CreateColorAttachmentReference(){
    VkAttachmentReference color_attachment_reference{};

    color_attachment_reference.attachment = 0;
    color_attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    return color_attachment_reference;
}

VkSubpassDependency CtGraphicsPipeline::CreateSubpassDependency(){
    VkSubpassDependency dependency{};

    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;

    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;   

    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    return dependency;
}

VkAttachmentDescription CtGraphicsPipeline::CreateDepthAttachmentDescription(VkPhysicalDevice* physical_device){
    VkAttachmentDescription depth_attachment{};

    depth_attachment.format = FindDepthFormat(physical_device);
    depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    return depth_attachment;
}

VkAttachmentReference CtGraphicsPipeline::CreateDepthAttachmentReference(){
    VkAttachmentReference depth_attachment_reference{};

    depth_attachment_reference.attachment = 1;
    depth_attachment_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    return depth_attachment_reference;
}

VkSubpassDescription CtGraphicsPipeline::CreateSubpassDescription(VkAttachmentReference& color_attachment_reference, VkAttachmentReference& depth_attachment_reference){
    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_attachment_reference;

    subpass.pDepthStencilAttachment = &depth_attachment_reference;

    subpass.inputAttachmentCount = 0;
    subpass.pInputAttachments = nullptr;
    subpass.pResolveAttachments = nullptr;
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

    return subpass;
}

//Let's define our dynamic states
std::vector<VkDynamicState> dynamic_states = {
    VK_DYNAMIC_STATE_VIEWPORT,
    VK_DYNAMIC_STATE_SCISSOR
};

VkPipelineDynamicStateCreateInfo CtGraphicsPipeline::CreateDynamicState(){
    VkPipelineDynamicStateCreateInfo dynamic_state_create_info {};

    //Define our structure type
    dynamic_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;

    //Then let's feed our struct the information 
    dynamic_state_create_info.dynamicStateCount = static_cast<uint32_t>(dynamic_states.size());
    dynamic_state_create_info.pDynamicStates = dynamic_states.data();

    return dynamic_state_create_info;
}

VkFormat CtGraphicsPipeline::FindSupportedFormat(VkPhysicalDevice* physical_device, const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features){

    for (VkFormat format : candidates){
        VkFormatProperties properties;
        vkGetPhysicalDeviceFormatProperties(*physical_device, format, &properties);

        if (tiling == VK_IMAGE_TILING_LINEAR && (properties.linearTilingFeatures & features) == features){
            return format;
        }
        else 
        if (tiling == VK_IMAGE_TILING_OPTIMAL && (properties.optimalTilingFeatures & features) == features){
            return format;
        }
    }
    throw std::runtime_error("Failed to find a supported format!");
}

VkFormat CtGraphicsPipeline::FindDepthFormat(VkPhysicalDevice* physical_device){
    return FindSupportedFormat(physical_device, {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
                                VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}