#include <vulkan/vulkan.h>
#include "CtQueueFamily.h"
#include <vector>

bool CtQueueFamily::IsComplete(){
    return graphics_family.has_value() && present_family.has_value();
}

void CtQueueFamily::ImplementQueueFamily(VkPhysicalDevice device){
    //First let's grab the queue families from our device
    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);

    std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families.data());

    //Now let's find our queue families
    int i = 0;
    for(const auto& queue_family_index : queue_families){
        if(queue_family_index.queueFlags & VK_QUEUE_GRAPHICS_BIT){
            graphics_family = i;
        }

        VkBool32 present_support = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &present_support);

        if(present_support){
            present_family = i;
        }

        if(IsComplete()){
            break;
        }
        
        i++;
    }
}

bool CtQueueFamily::TestDevice(VkPhysicalDevice device){
    //First let's grab the queue families from our device
    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);

    std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families.data());

    //Now let's find our queue families
    int i = 0;
    bool has_graphics = false;
    bool has_present = false;
    for(const auto& queue_family_index : queue_families){
        if(queue_family_index.queueFlags & VK_QUEUE_GRAPHICS_BIT){
            has_graphics = true;
        }

        VkBool32 present_support = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &present_support);

        if(present_support){
            has_present = true;
        }

        if(has_graphics && has_present){
            return true;
        }
        
        i++;
    }

    return false;
}

//Let's create our queue families here
CtQueueFamily* CtQueueFamily::CreateQueueFamily(VkSurfaceKHR* surface){

    CtQueueFamily* queue_family = new CtQueueFamily();

    queue_family->surface = *surface;

    return queue_family;
}

void CtQueueFamily::PopulateQueueFamilyCreate(CtDeviceQueueCreateInfo& create_info, const void* pNext,
            VkDeviceQueueCreateFlags flags, uint32_t queue_family_index, uint32_t queue_count,
            const float* pointer_to_queue_priorites){
    create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    create_info.pNext = pNext;
    create_info.flags = flags;
    create_info.queueFamilyIndex = queue_family_index;
    create_info.queueCount = queue_count;
    create_info.pQueuePriorities = pointer_to_queue_priorites;
}

void CtQueueFamily::TransferQueueFamilyCreate(CtDeviceQueueCreateInfo& ct_queue_create_info, VkDeviceQueueCreateInfo& vk_queue_create_info){
    vk_queue_create_info.sType = ct_queue_create_info.sType;
    vk_queue_create_info.pNext = ct_queue_create_info.pNext;
    vk_queue_create_info.flags = ct_queue_create_info.flags;
    vk_queue_create_info.queueFamilyIndex = ct_queue_create_info.queueFamilyIndex;
    vk_queue_create_info.queueCount = ct_queue_create_info.queueCount;
    vk_queue_create_info.pQueuePriorities = ct_queue_create_info.pQueuePriorities;
}