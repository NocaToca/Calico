#include "CtRenderer.h"
#include "Engine.h"
#include <vulkan/vulkan.h>
#include <stdexcept>
#include "CtDevice.h"
#include "CtSwapchain.h"
#include "CtGraphicsPipeline.h"
#include "CtVertex.h"
#include "CtQueueFamily.h"

void CtRenderer::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer, VkDeviceMemory &buffer_memory){
    VkDevice interface_device = *(device->GetInterfaceDevice());

    VkBufferCreateInfo buffer_info{};
    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size = size;
    buffer_info.usage = usage;
    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if(vkCreateBuffer(interface_device, &buffer_info, nullptr, &buffer) != VK_SUCCESS){
        throw std::runtime_error("Failed to create buffer.");
    }

    VkMemoryRequirements memory_requirements;
    vkGetBufferMemoryRequirements(interface_device, buffer, &memory_requirements);

    VkMemoryAllocateInfo allocate_info {};
    allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocate_info.allocationSize = memory_requirements.size;
    allocate_info.memoryTypeIndex = device->FindMemoryType(memory_requirements.memoryTypeBits, properties);

    if(vkAllocateMemory(interface_device, &allocate_info, nullptr, &buffer_memory) != VK_SUCCESS){
        throw std::runtime_error("Failed to allocate memory buffer");
    }

    vkBindBufferMemory(interface_device, buffer, buffer_memory, 0);
}

void CtRenderer::CopyBuffer(VkBuffer source_buffer, VkBuffer destination_buffer, VkDeviceSize size){
    VkDevice interface_device = *(device->GetInterfaceDevice());

    VkCommandBuffer command_buffer = BeginSingleTimeCommands();

    VkBufferCopy copy_region{};
    copy_region.srcOffset = 0;
    copy_region.dstOffset = 0;
    copy_region.size = size;

    vkCmdCopyBuffer(command_buffer, source_buffer, destination_buffer, 1, &copy_region);

    EndSingleTimeCommands(command_buffer);
}

VkCommandBuffer CtRenderer::BeginSingleTimeCommands(){
    VkDevice interface_device = *(device->GetInterfaceDevice());

    VkCommandBufferAllocateInfo allocate_info{};
    allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocate_info.commandPool = command_pool;
    allocate_info.commandBufferCount = 1;

    VkCommandBuffer command_buffer;
    vkAllocateCommandBuffers(interface_device, &allocate_info, &command_buffer);

    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(command_buffer, &begin_info);

    return command_buffer;
}

void CtRenderer::EndSingleTimeCommands(VkCommandBuffer command_buffer){
    VkDevice interface_device = *(device->GetInterfaceDevice());

    vkEndCommandBuffer(command_buffer);

    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffer;

    vkQueueSubmit(device->queue_family->graphics_queue, 1, &submit_info, VK_NULL_HANDLE);
    vkQueueWaitIdle(device->queue_family->graphics_queue);

    vkFreeCommandBuffers(interface_device, command_pool, 1, &command_buffer);
}

void CtRenderer::CreateVertexBuffer(){
    VkDevice interface_device = *(device->GetInterfaceDevice());

    VkDeviceSize buffer_size = sizeof(test_vertices[0]) * test_vertices.size();

    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;
    CreateBuffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, staging_buffer, staging_buffer_memory);

    void *data;
    vkMapMemory(interface_device, staging_buffer_memory, 0, buffer_size, 0, &data);
    memcpy(data, test_vertices.data(), (size_t)buffer_size);
    vkUnmapMemory(interface_device, staging_buffer_memory);

    CreateBuffer(buffer_size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertex_buffer, vertex_buffer_memory);

    CopyBuffer(staging_buffer, vertex_buffer, buffer_size);

    vkDestroyBuffer(interface_device, staging_buffer, nullptr);
    vkFreeMemory(interface_device, staging_buffer_memory, nullptr);

    printf("Created Vertex Buffer.\n");
}

void CtRenderer::CreateIndexBuffer(){
    VkDevice interface_device = *(device->GetInterfaceDevice());

    VkDeviceSize buffer_size = sizeof(test_indices[0]) * test_indices.size();

    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;
    CreateBuffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, staging_buffer, staging_buffer_memory);

    void *data;
    vkMapMemory(interface_device, staging_buffer_memory, 0, buffer_size, 0, &data);
    memcpy(data, test_indices.data(), (size_t)buffer_size);
    vkUnmapMemory(interface_device, staging_buffer_memory);

    CreateBuffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, index_buffer, index_buffer_memory);

    CopyBuffer(staging_buffer, index_buffer, buffer_size);

    vkDestroyBuffer(interface_device, staging_buffer, nullptr);
    vkFreeMemory(interface_device, staging_buffer_memory, nullptr);

    printf("Created Index Buffer.\n");
}

//This probably pulls from the most external classes
void CtRenderer::RecordCommandBuffer(VkCommandBuffer command_buffer, uint32_t image_index){

    //Let's start creating the command buffer
    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = 0;
    begin_info.pInheritanceInfo = nullptr;

    if(vkBeginCommandBuffer(command_buffer, &begin_info) != VK_SUCCESS){
        throw std::runtime_error("Failed to begin recording to command buffer.");
    }

    VkRenderPassBeginInfo render_pass_info{};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_info.renderPass = graphics_pipeline->render_pass;
    render_pass_info.framebuffer = swapchain->swapchain_framebuffers[image_index];
    render_pass_info.renderArea.offset = {0, 0};
    render_pass_info.renderArea.extent = swapchain->swapchain_extent;

    //The clear values
    std::array<VkClearValue, 2> clear_values{};
    clear_values[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
    clear_values[1].depthStencil = {1.0f, 0};

    render_pass_info.clearValueCount = static_cast<uint32_t>(clear_values.size());
    render_pass_info.pClearValues = clear_values.data();

    vkCmdBeginRenderPass(command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(swapchain->swapchain_extent.width);
    viewport.height = static_cast<float>(swapchain->swapchain_extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(command_buffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = swapchain->swapchain_extent;
    vkCmdSetScissor(command_buffer, 0, 1, &scissor);

    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline->graphics_pipeline);

    VkBuffer vertex_buffers[] = {vertex_buffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(command_buffer, 0, 1, vertex_buffers, offsets);

    vkCmdBindIndexBuffer(command_buffer, index_buffer, 0, VK_INDEX_TYPE_UINT16);

    // vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline->pipeline_layout, 0, 1, &descriptor_sets[current_frame], 0, nullptr);

    vkCmdDrawIndexed(command_buffer, static_cast<uint16_t>(test_indices.size()), 1, 0, 0, 0);

    vkCmdEndRenderPass(command_buffer);

    if (vkEndCommandBuffer(command_buffer) != VK_SUCCESS){
        throw std::runtime_error("failed to record command buffer!");
    }
}