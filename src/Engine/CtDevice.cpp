#include "CtDevice.h"
#include "CtInstance.h"
#include <cstdint>
#include <vector>
#include "Engine.h"
#include <set>
#include <string>

CtDevice* CtDevice::CreateDevice(Engine* ct_engine, EngineSettings settings){
    CtDevice* ct_device = new CtDevice();

    CtDeviceRequirments requirements {};
    FillCtDeviceRequirements(requirements, VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU, true);

    ct_device->ChooseDevice(*(ct_engine->instance), requirements);

    return ct_device;

}

void CtDevice::FillCtDeviceRequirements(CtDeviceRequirments& device_requirements, VkPhysicalDeviceType device_type, bool has_sampler_anisotropy){
    device_requirements.device_type = device_type;
    device_requirements.has_sampler_anisotropy = has_sampler_anisotropy;
}

std::vector<const char*> CtDevice::GetRequiredDeviceExtensions(){
    std::vector<const char*> device_extensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    return device_extensions;
}

bool CtDevice::CheckDeviceExtensionSupport(VkPhysicalDevice device){
    
    //To first check to see if we can even support the features we're looking for, we must first grab what features we can support
    uint32_t extension_count;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, nullptr);

    std::vector<VkExtensionProperties> supported_features(extension_count);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, supported_features.data());

    std::vector<const char*> required_features = GetRequiredDeviceExtensions();
    std::set<std::string> required_extensions(required_features.begin(), required_features.end());

    for(const auto& extension : supported_features){
        required_extensions.erase(extension.extensionName);
    }

    return required_extensions.empty();
}

bool CtDevice::IsDeviceSuitable(VkPhysicalDevice device, CtDeviceRequirments requirements){
    VkPhysicalDeviceProperties device_properties;
    VkPhysicalDeviceFeatures device_features;

    vkGetPhysicalDeviceProperties(device, &device_properties);
    vkGetPhysicalDeviceFeatures(device, &device_features);

    //Once we get to queue families and swap chains we will implement those, but we need to check that we can use the swap chain extensions
    bool extensions_supported = CheckDeviceExtensionSupport(device);

    return  device_properties.deviceType == requirements.device_type &&
            device_features.samplerAnisotropy == requirements.has_sampler_anisotropy &&
            extensions_supported;

}

void CtDevice::ChooseDevice(CtInstance ct_instance, CtDeviceRequirments requirements){

    //Let's go through and choose a GPU
    uint32_t device_count = 0;

    vkEnumeratePhysicalDevices(*(ct_instance.GetInstance()), &device_count, nullptr);

    //If there are no devices we cannot continue :c
    if(device_count == 0){
        throw std::runtime_error("Failed to find a GPU.");
    }
    
    //Let's grab all of our devices
    std::vector<VkPhysicalDevice> devices(device_count);
    vkEnumeratePhysicalDevices(*(ct_instance.GetInstance()), &device_count, devices.data());

    //Now let's check to see if any of the devices are suitable
    for(const auto& device : devices){
        if(IsDeviceSuitable(device, requirements)){
            physical_device = device;
            break;
        }
    }

    //Then we just want to check if we actually found a device before continuing
    if(physical_device == VK_NULL_HANDLE){
        throw std::runtime_error("Failed to find a compatible GPU.");
    } else {
        
        //Now I just want to print the name so I know what GPU we are using
        VkPhysicalDeviceProperties properties {};
        vkGetPhysicalDeviceProperties(physical_device, &properties);

        printf("Found physical device: %s.\n", properties.deviceName);
    }
}