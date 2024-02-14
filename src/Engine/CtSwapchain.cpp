#include "CtSwapchain.h"
#include "CtDevice.h"
#include <limits>
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include "CtWindow.h"
#include "CtQueueFamily.h"
#include <algorithm>
#include "Engine.h"
#include <stdexcept>

//Since this function is static, I'm not going to use CtImageViewCreateInfo since that is not
VkImageView CtSwapchain::CreateImageView(CtDevice* device, VkImage image, VkFormat format, VkImageAspectFlags aspect_flags){

    VkImageViewCreateInfo image_view_create_info {};
    image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    image_view_create_info.image = image;
    image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    image_view_create_info.format = format;
    image_view_create_info.subresourceRange.aspectMask = aspect_flags;
    image_view_create_info.subresourceRange.baseMipLevel = 0;
    image_view_create_info.subresourceRange.levelCount = 1;
    image_view_create_info.subresourceRange.baseArrayLayer = 0;
    image_view_create_info.subresourceRange.layerCount = 1;

    VkImageView our_image_view;

    if(vkCreateImageView(*(device->GetInterfaceDevice()), &image_view_create_info, nullptr, &our_image_view) != VK_SUCCESS){
        throw std::runtime_error("Failed to create image views.");
    }

    return our_image_view;
}

void CtSwapchain::PopulateImageViewCreateInfo(CtImageViewCreateInfo& create_info,
        const void* pointer_to_next, VkImageViewCreateFlags flags, VkImage image, VkImageViewType view_type,
        VkFormat format, VkComponentMapping components, VkImageSubresourceRange subresource_range){
    create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    create_info.pNext = pointer_to_next;
    create_info.flags = flags;
    create_info.image = image;
    create_info.viewType = view_type;
    create_info.format = format;
    create_info.components = components;
    create_info.subresourceRange = subresource_range;
}

void CtSwapchain::TransferImageViewCreateInfo(CtImageViewCreateInfo& ct_image_view_create_info, VkImageViewCreateInfo& vk_image_view_create_info){
    vk_image_view_create_info.sType = ct_image_view_create_info.sType;
    vk_image_view_create_info.pNext = ct_image_view_create_info.pNext;
    vk_image_view_create_info.flags = ct_image_view_create_info.flags;
    vk_image_view_create_info.image = ct_image_view_create_info.image;
    vk_image_view_create_info.viewType = ct_image_view_create_info.viewType;
    vk_image_view_create_info.format = ct_image_view_create_info.format;
    vk_image_view_create_info.components = ct_image_view_create_info.components;
    vk_image_view_create_info.subresourceRange = ct_image_view_create_info.subresourceRange;
}

void CtSwapchain::InitializeSwapchainImageViews(){
    size_t swapchain_images_count = swapchain_images.size();

    swapchain_image_views.resize(swapchain_images_count);

    for(size_t i = 0; i < swapchain_images_count; i++){
        CtImageViewCreateInfo ct_create_info {};
        VkComponentMapping components {};
        components.r = VK_COMPONENT_SWIZZLE_IDENTITY; components.g = VK_COMPONENT_SWIZZLE_IDENTITY; components.b = VK_COMPONENT_SWIZZLE_IDENTITY; components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        VkImageSubresourceRange resource_range {};
        resource_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; resource_range.baseMipLevel = 0; resource_range.levelCount = 1; resource_range.baseArrayLayer = 0; resource_range.layerCount = 1;

        VkImageViewCreateInfo vk_create_info {};
        PopulateImageViewCreateInfo(ct_create_info, nullptr, 0, swapchain_images[i], VK_IMAGE_VIEW_TYPE_2D,
            swapchain_image_format, components, resource_range);
        TransferImageViewCreateInfo(ct_create_info, vk_create_info);

        if(vkCreateImageView(*(device->GetInterfaceDevice()), &vk_create_info, nullptr, &swapchain_image_views[i]) != VK_SUCCESS){
            throw std::runtime_error("Failed to create image views.");
        }
    }

    printf("Finished creating image views.\n");
}

CtSwapchainSupportDetails CtSwapchain::QuerySwapchainSupport(VkPhysicalDevice physical_device, VkSurfaceKHR* surface){

    CtSwapchainSupportDetails support_details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, *surface, &support_details.capabilities);

    uint32_t format_count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, *surface, &format_count, nullptr);

    //If we actually have formats, we should fill our vector
    if(format_count != 0){
        support_details.formats.resize(format_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, *surface, &format_count, support_details.formats.data());
    }

    //Same idea for the present counts
    uint32_t present_mode_count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, *surface, &present_mode_count, nullptr);

    //Same thing, only do work if we can have modes
    if(present_mode_count != 0){
        support_details.present_modes.resize(present_mode_count);
        vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, *surface, &present_mode_count, support_details.present_modes.data());
    }

    return support_details;
}

CtSwapchain* CtSwapchain::CreateSwapchain(Engine* ct_engine){

    CtSwapchainSupportDetails swap_chain_support_details = QuerySwapchainSupport(*(ct_engine->devices->GetPhysicalDevice()), ct_engine->window->GetSurface());

    CtSwapchain* swapchain = new CtSwapchain();
    swapchain->device = ct_engine->devices; 
    swapchain->window = ct_engine->window;
    swapchain->InitializeSwapchain(swap_chain_support_details);
    swapchain->InitializeSwapchainImageViews();

    return swapchain;
}

VkSurfaceFormatKHR CtSwapchain::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> surface_formats){
    for(const auto& surface_format : surface_formats){
        if(surface_format.format == VK_FORMAT_B8G8R8A8_SRGB && surface_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR){
            return surface_format;
        }
    }

    return surface_formats[0];
}

VkPresentModeKHR CtSwapchain::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& present_modes){
    for(const auto& present_mode : present_modes){

        //If we can, we would want the presentation engine to wait for the next vertical blank period to help prevent screen tears
        if(present_mode == VK_PRESENT_MODE_MAILBOX_KHR){
            return present_mode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D CtSwapchain::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities){
    if(capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()){
        return capabilities.currentExtent;
    } else {
        int width, height;
        glfwGetFramebufferSize(window->GetWindow(), &width, &height);

        VkExtent2D actual_extent ={
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        actual_extent.width = std::clamp(actual_extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actual_extent.height = std::clamp(actual_extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actual_extent;
    }
}

void CtSwapchain::InitializeSwapchain(CtSwapchainSupportDetails support_details){
    VkSurfaceFormatKHR surface_format = ChooseSwapSurfaceFormat(support_details.formats);
    VkPresentModeKHR present_mode = ChooseSwapPresentMode(support_details.present_modes);
    VkExtent2D extent = ChooseSwapExtent(support_details.capabilities);

    uint32_t image_count = support_details.capabilities.minImageCount + 1;

    if(support_details.capabilities.maxImageCount > 0 && image_count > support_details.capabilities.maxImageCount){
        image_count = support_details.capabilities.maxImageCount;
    }

    CtQueueFamily* queue_family = device->queue_family;

    uint32_t graphics_family_index = queue_family->GraphicsFamilyValue();
    uint32_t present_family_index = queue_family->PresentFamilyValue();

    uint32_t queue_family_indicies[] = {graphics_family_index, present_family_index};

    VkSharingMode image_sharing_mode;
    uint32_t queue_family_index_count;
    uint32_t* pointer_to_queue_family_indices = nullptr;

    if(graphics_family_index != present_family_index){
        image_sharing_mode = VK_SHARING_MODE_CONCURRENT;
        queue_family_index_count = 2;
        pointer_to_queue_family_indices = queue_family_indicies;
    } else {
        image_sharing_mode = VK_SHARING_MODE_EXCLUSIVE;
        queue_family_index_count = 0;
        pointer_to_queue_family_indices = nullptr;
    }

    CtSwapchainCreateInfoKHR ct_create_info {};
    PopulateSwapchainCreateInfo(ct_create_info,
        nullptr, 0, *(window->GetSurface()), image_count, surface_format.format, surface_format.colorSpace,
        extent, 1, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        image_sharing_mode, queue_family_index_count, pointer_to_queue_family_indices,
        support_details.capabilities.currentTransform, VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        present_mode, VK_TRUE, VK_NULL_HANDLE);
    
    VkSwapchainCreateInfoKHR vk_create_info {};
    TransferSwapchainCreateInfo(ct_create_info, vk_create_info);

    if(vkCreateSwapchainKHR(*(device->GetInterfaceDevice()), &vk_create_info, nullptr, &swapchain) != VK_SUCCESS){
        throw std::runtime_error("Failed to create a swapchain");
    }

    vkGetSwapchainImagesKHR(*(device->GetInterfaceDevice()), swapchain, &image_count, nullptr);
    swapchain_images.resize(image_count);
    vkGetSwapchainImagesKHR(*(device->GetInterfaceDevice()), swapchain, &image_count, swapchain_images.data());

    swapchain_image_format = surface_format.format;
    swapchain_extent = extent;

    printf("Created the swapchain.\n");
}

void CtSwapchain::TransferSwapchainCreateInfo(CtSwapchainCreateInfoKHR& ct_swapchain_create_info, VkSwapchainCreateInfoKHR& vk_swapchain_create_info){
    vk_swapchain_create_info.sType = ct_swapchain_create_info.sType;
    vk_swapchain_create_info.pNext = ct_swapchain_create_info.pNext;
    vk_swapchain_create_info.flags = ct_swapchain_create_info.flags;
    vk_swapchain_create_info.surface = ct_swapchain_create_info.surface;
    vk_swapchain_create_info.minImageCount = ct_swapchain_create_info.minImageCount;
    vk_swapchain_create_info.imageFormat = ct_swapchain_create_info.imageFormat;
    vk_swapchain_create_info.imageColorSpace = ct_swapchain_create_info.imageColorSpace;
    vk_swapchain_create_info.imageExtent = ct_swapchain_create_info.imageExtent;
    vk_swapchain_create_info.imageArrayLayers = ct_swapchain_create_info.imageArrayLayers;
    vk_swapchain_create_info.imageUsage = ct_swapchain_create_info.imageUsage;
    vk_swapchain_create_info.imageSharingMode = ct_swapchain_create_info.imageSharingMode;
    vk_swapchain_create_info.queueFamilyIndexCount = ct_swapchain_create_info.queueFamilyIndexCount;
    vk_swapchain_create_info.pQueueFamilyIndices = ct_swapchain_create_info.pQueueFamilyIndices;
    vk_swapchain_create_info.preTransform = ct_swapchain_create_info.preTransform;
    vk_swapchain_create_info.compositeAlpha = ct_swapchain_create_info.compositeAlpha;
    vk_swapchain_create_info.presentMode = ct_swapchain_create_info.presentMode;
    vk_swapchain_create_info.clipped = ct_swapchain_create_info.clipped;
    vk_swapchain_create_info.oldSwapchain = ct_swapchain_create_info.oldSwapchain;

}

void CtSwapchain::PopulateSwapchainCreateInfo(CtSwapchainCreateInfoKHR& create_info, 
            const void* pointer_to_next, VkSwapchainCreateFlagsKHR flags, VkSurfaceKHR surface, uint32_t min_image_count,
            VkFormat image_format, VkColorSpaceKHR image_color_space, VkExtent2D image_extent, uint32_t image_array_layers,
            VkImageUsageFlags image_usage, VkSharingMode image_sharing_mode, uint32_t queue_family_index_count, const uint32_t* pointer_to_queue_family_indicies,
            VkSurfaceTransformFlagBitsKHR pre_transform, VkCompositeAlphaFlagBitsKHR composite_alpha, VkPresentModeKHR present_mode,
            VkBool32 clipped, VkSwapchainKHR old_swapchain){

    create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.pNext = pointer_to_next;
    create_info.flags = flags;
    create_info.surface = surface;
    create_info.minImageCount = min_image_count;
    create_info.imageFormat = image_format;
    create_info.imageColorSpace = image_color_space;
    create_info.imageExtent = image_extent;
    create_info.imageArrayLayers = image_array_layers;
    create_info.imageUsage = image_usage;
    create_info.imageSharingMode = image_sharing_mode;
    create_info.queueFamilyIndexCount = queue_family_index_count;
    create_info.pQueueFamilyIndices = pointer_to_queue_family_indicies;
    create_info.preTransform = pre_transform;
    create_info.compositeAlpha = composite_alpha;
    create_info.presentMode = present_mode;
    create_info.clipped = clipped;
    create_info.oldSwapchain = old_swapchain;
}
