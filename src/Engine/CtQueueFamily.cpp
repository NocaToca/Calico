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