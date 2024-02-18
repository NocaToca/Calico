#include "CtRenderer.h"
#include "Engine.h"
#include <vulkan/vulkan.h>
#include <stdexcept>
#include "CtDevice.h"
#include "CtSwapchain.h"
#include "CtGraphicsPipeline.h"
#include "CtVertex.h"

void CtSwapchain::CreateDepthResources(){

    VkFormat depth_format = CtGraphicsPipeline::FindDepthFormat(device->GetPhysicalDevice());

    CreateImage(swapchain_extent.width, swapchain_extent.height, depth_format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depth_image, depth_image_memory);

    depth_image_view = CreateImageView(depth_image, depth_format, VK_IMAGE_ASPECT_DEPTH_BIT);   

    TransitionImageLayout(depth_image, depth_format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

    printf("Created depth resources.\n");
}

VkImageView CtSwapchain::CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspect_flags){
    VkImageViewCreateInfo view_info{};
    view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_info.image = image;
    view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_info.format = format;
    view_info.subresourceRange.aspectMask = aspect_flags;
    view_info.subresourceRange.baseMipLevel = 0;
    view_info.subresourceRange.levelCount = 1;
    view_info.subresourceRange.baseArrayLayer = 0;
    view_info.subresourceRange.layerCount = 1;

    VkImageView image_view;
    if(vkCreateImageView(*(device->GetInterfaceDevice()), &view_info, nullptr, &image_view) != VK_SUCCESS){
        throw std::runtime_error("Failed to create texture image view");
    }

    return image_view;
}

void CtSwapchain::CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& image_memory){
    VkDevice interface_device = *(device->GetInterfaceDevice());

    VkImageCreateInfo image_info{};
    image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.extent.width = width;
    image_info.extent.height = height;
    image_info.extent.depth = 1;
    image_info.mipLevels = 1;
    image_info.arrayLayers = 1;

    image_info.format = format;
    image_info.tiling = tiling;
    image_info.usage = usage;

    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        
    image_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_info.flags = 0;

    VkResult result = vkCreateImage(interface_device, &image_info, nullptr, &image);
    switch(result){
        case VK_SUCCESS:
            //do nothing
            break;
        case VK_ERROR_OUT_OF_MEMORY:
            throw std::runtime_error("Failure to create image. Host is out of memory.\n");
        case VK_ERROR_OUT_OF_DEVICE_MEMORY:
            throw std::runtime_error("Failure to create image. Device is out of memory.\n");
        case VK_ERROR_COMPRESSION_EXHAUSTED_EXT:
            throw std::runtime_error("Failure to create image. Compression exhuasted.\n");
        default:
            throw std::runtime_error("Failure to create image.\n")

    }

    VkMemoryRequirements memory_requirements;
    vkGetImageMemoryRequirements(interface_device, image, &memory_requirements);

    VkMemoryAllocateInfo allocate_info {};
    allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocate_info.allocationSize = memory_requirements.size;
    allocate_info.memoryTypeIndex = device->FindMemoryType(memory_requirements.memoryTypeBits, properties);

    result = vkAllocateMemory(interface_device, &allocate_info, nullptr, &image_memory);

    switch(result){
        case VK_SUCCESS:
            //do nothing
            break;
        case VK_ERROR_OUT_OF_HOST_MEMORY:
            throw std::runtime_error("Failure to allocate memory. Host is out of memory.\n");
            break;
        case VK_ERROR_OUT_OF_DEVICE_MEMORY:
            throw std::runtime_error("Failure to allocate memory. Device is out of memory.\n");
            break;
        case VK_ERROR_INVALID_EXTERNAL_HANDLE:
            throw std::runtime_error("Failure to allocate memory. Handler is invalid.\n");
            break;
        default:
            throw std::runtime_error("Failed to allocate memory.\n");
            break;
    }

    vkBindImageMemory(interface_device, image, image_memory, 0);
}

bool HasStencilComponent(VkFormat format){
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

void CtSwapchain::TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout){
    VkCommandBuffer command_buffer = renderer->BeginSingleTimeCommands();

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = old_layout;
    barrier.newLayout = new_layout;

    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = 0;

    VkPipelineStageFlags source_stage;
    VkPipelineStageFlags destination_stage;

    if(new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL){
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

        if(HasStencilComponent(format)){
            barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
    } else {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }

    if(old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL){
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else
    if(old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL){
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else 
    if(old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL){
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destination_stage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;

    } else {
        throw std::invalid_argument("Unsupported layout transition");
    }

    vkCmdPipelineBarrier(command_buffer, source_stage, destination_stage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

    renderer->EndSingleTimeCommands(command_buffer);
}