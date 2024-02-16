#include "CtRenderer.h"
#include "Engine.h"
#include <vulkan/vulkan.h>
#include <stdexcept>
#include "CtDevice.h"
#include "CtSwapchain.h"
#include "CtGraphicsPipeline.h"
#include "CtQueueFamily.h"

CtRenderer* CtRenderer::CreateRenderer(EngineSettings settings, CtDevice* device, CtSwapchain* swapchain, CtGraphicsPipeline* graphics_pipeline){

    CtRenderer* ct_renderer = new CtRenderer();

    ct_renderer->swapchain = swapchain;
    ct_renderer->device = device;
    ct_renderer->graphics_pipeline = graphics_pipeline;
    ct_renderer->max_frames_in_flight = settings.graphics_settings.max_frames_in_flight;
    ct_renderer->current_frame = 0;
    ct_renderer->CreateSyncObjects();
    ct_renderer->CreateCommandPool();
    ct_renderer->CreateCommandBuffers();
    ct_renderer->CreateIndexBuffer();
    ct_renderer->CreateVertexBuffer();
    swapchain->renderer = ct_renderer;
    swapchain->CreateDepthResources();
    swapchain->InitializeSwapchainFramebuffers(graphics_pipeline->render_pass);

    printf("Created Renderer.\n");
    return ct_renderer;
}

//Our actual drawing function
void CtRenderer::DrawFrame(){
    VkDevice interface_device = *(device->GetInterfaceDevice());
    VkSwapchainKHR swapchain_khr = (swapchain->swapchain);
    VkQueue present_queue = (device->queue_family->present_queue);
    VkQueue graphics_queue = device->queue_family->graphics_queue;

    vkWaitForFences(interface_device, 1, &in_flight_fences[current_frame], VK_TRUE, UINT64_MAX);

    uint32_t image_index;
    //First we have to wait
    VkResult result = vkAcquireNextImageKHR(interface_device, swapchain->swapchain, UINT64_MAX, image_available_semaphores[current_frame], VK_NULL_HANDLE, &image_index);

    //Let's see if we need to change our swap chain
    if(result == VK_ERROR_OUT_OF_DATE_KHR){
        swapchain->RecreateSwapchain(graphics_pipeline->render_pass);
        return;
    } else 
    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR){
        throw std::runtime_error("Failed to acquire swap chain image!");
    }

    vkResetFences(interface_device, 1, &in_flight_fences[current_frame]);

    vkResetCommandBuffer(command_buffers[current_frame], 0);
    RecordCommandBuffer(command_buffers[current_frame], image_index);

    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore wait_semaphores[] = {image_available_semaphores[current_frame]};
    VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = wait_semaphores;
    submit_info.pWaitDstStageMask = wait_stages;

    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffers[current_frame];

    VkSemaphore signal_semaphore[] = {render_finished_semaphores[current_frame]};
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = signal_semaphore;

    if (vkQueueSubmit(graphics_queue, 1, &submit_info, in_flight_fences[current_frame]) != VK_SUCCESS){
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    VkPresentInfoKHR present_info{};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = signal_semaphore;

    VkSwapchainKHR swapchains[] = {swapchain->swapchain};
    present_info.swapchainCount = 1;
    present_info.pSwapchains = swapchains;
    present_info.pImageIndices = &image_index;
    present_info.pResults = nullptr;

    result = vkQueuePresentKHR(present_queue, &present_info);

    // Let's re-query to see if our result is suboptimal mostly (or failed)
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebuffer_resized){
        framebuffer_resized = false;
        swapchain->RecreateSwapchain(graphics_pipeline->render_pass);
    }
    else 
    if (result != VK_SUCCESS){
        throw std::runtime_error("Could not present swap chain image.");
    }

    current_frame = (current_frame + 1) % max_frames_in_flight;
}

void CtRenderer::CreateSyncObjects(){

    image_available_semaphores.resize(max_frames_in_flight);
    render_finished_semaphores.resize(max_frames_in_flight);
    in_flight_fences.resize(max_frames_in_flight);

    //Let's create our create info!
    VkSemaphoreCreateInfo semaphore_create_info {};
    semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    //And then our fence! We want to start signaled!
    VkFenceCreateInfo fence_create_info {};
    fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    //And then let's fill our vectors :3
    for(size_t i = 0; i < max_frames_in_flight; i++){
        if (vkCreateSemaphore(*(device->GetInterfaceDevice()), &semaphore_create_info, nullptr, &image_available_semaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(*(device->GetInterfaceDevice()), &semaphore_create_info, nullptr, &render_finished_semaphores[i]) != VK_SUCCESS ||
            vkCreateFence(*(device->GetInterfaceDevice()), &fence_create_info, nullptr, &in_flight_fences[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create semaphores!");
        }
    }

    printf("Created Sync Objects!\n");
}

void CtRenderer::CreateCommandBuffers(){
    command_buffers.resize(max_frames_in_flight);

    VkCommandBufferAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.commandPool = command_pool;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandBufferCount = (uint32_t)command_buffers.size();

    if(vkAllocateCommandBuffers(*(device->GetInterfaceDevice()), &alloc_info, command_buffers.data()) != VK_SUCCESS){
        throw std::runtime_error("Failed to create command buffers");
    }

    printf("Created Command Buffers.\n");
}

void CtRenderer::CreateCommandPool(){
    //This is honestly super simple, we just mostly need the graphics queue
    VkCommandPoolCreateInfo pool_info{};
    pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
     pool_info.queueFamilyIndex = device->queue_family->graphics_family.value();

    if(vkCreateCommandPool(*(device->GetInterfaceDevice()), &pool_info, nullptr, &command_pool) != VK_SUCCESS){
        throw std::runtime_error("Failed to create a command pool");
    }

    printf("Created Command Pool.\n");
}