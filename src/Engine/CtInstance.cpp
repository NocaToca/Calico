#include "CtInstance.h"
#include <stdexcept>
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include <cstdint>
#include <iostream>
#include <cstring>

CtInstance::CtInstance(){
    is_debug_instance = false;
}
CtInstance::~CtInstance() = default;

void CtInstance::CreateApplicationInfo(CtInstanceApplicationInfo& app_info, const void* pointer_next, const char* pointer_application_name, uint32_t application_version, const char* pointer_engine_name,
            uint32_t engine_version, uint32_t api_version){

    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pNext = pointer_next;
    app_info.pApplicationName = pointer_application_name;
    app_info.applicationVersion = application_version;
    app_info.pEngineName = pointer_engine_name;
    app_info.engineVersion = engine_version;
    app_info.apiVersion = api_version;
}

void CtInstance::CreateInstanceInfo(CtInstanceCreateInfo& create_info, const void* pointer_next, VkInstanceCreateFlags flags, uint32_t enabled_layer_count,
            const char* const* pointer_of_pointer_enabled_layer_names, uint32_t enabled_extension_count, const char* const* pointer_of_pointer_enabled_extension_names){

    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pNext = pointer_next;
    create_info.flags = flags;
    create_info.enabledLayerCount = enabled_layer_count;
    create_info.ppEnabledLayerNames = pointer_of_pointer_enabled_layer_names;
    create_info.enabledExtensionCount = enabled_extension_count;
    create_info.ppEnabledExtensionNames = pointer_of_pointer_enabled_extension_names;

}

void CtInstance::CreateDebugMessengerInfo(CtInstanceDebugUtilsMessengerCreateInfo& create_info, const void* pointer_next, VkDebugUtilsMessengerCreateFlagsEXT flags, VkDebugUtilsMessageSeverityFlagsEXT message_severity,
            VkDebugUtilsMessageTypeFlagsEXT message_type, PFN_vkDebugUtilsMessengerCallbackEXT pointer_to_function_user_callback, void* user_data){
    create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    create_info.pNext = pointer_next;
    create_info.flags = flags;
    create_info.messageSeverity = message_severity;
    create_info.messageType = message_type;
    create_info.pfnUserCallback = pointer_to_function_user_callback;
    create_info.pUserData = user_data;

}

bool CtInstance::CheckValidationLayerSupport(std::vector<const char*> validation_layers){
    uint32_t layer_count;
    std::vector<VkLayerProperties> available_layers = {};
    vkEnumerateInstanceLayerProperties(&layer_count, nullptr); //This check returns the number of global layer properties, essentially
    //Since we don't know how many layers there are, we first actually need to do this check and then fill our array
    available_layers.resize(layer_count);
    vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

    //Let's just see if we can use our validation layer
    for(const char* layer_name : validation_layers){
        bool found_layer = false;

        for(const auto& layer_properties : available_layers){
            if(strcmp(layer_name, layer_properties.layerName) == 0){
                found_layer = true;
                break;
            }
        }

        if(!found_layer){
            return false;
        }
    }

    return true;
}

VkResult CtInstance::CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator,
    VkDebugUtilsMessengerEXT* pDebugMessenger){
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        if(func != nullptr){
            return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
        } else {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }
}

void CtInstance::SetupDebugInstance(){
    if(!is_debug_instance){
        throw std::runtime_error("Trying to set up a debug instance that is not a debug instance.");
    }

    CtInstanceDebugUtilsMessengerCreateInfo ct_debug_create_info {}; 
    
    CreateDebugMessengerInfo(ct_debug_create_info, nullptr, 0, 
    VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
    VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
    DebugCallback, nullptr);

    VkDebugUtilsMessengerCreateInfoEXT vk_create_info {};
    vk_create_info.sType = ct_debug_create_info.sType;
    vk_create_info.messageSeverity = ct_debug_create_info.messageSeverity;
    vk_create_info.messageType = ct_debug_create_info.messageType;
    vk_create_info.pfnUserCallback = ct_debug_create_info.pfnUserCallback;
    vk_create_info.pUserData = nullptr;

    //idk where the error returns are
    if(CreateDebugUtilsMessengerEXT(instance, &vk_create_info, nullptr, &debug_messenger) != VK_SUCCESS){
        throw std::runtime_error("Debug extension is not supported.");
    }

}

void CtInstance::EnableDebugInstance(CtInstanceCreateInfo &ct_create_info, std::vector<const char*> validation_layers){
    if(!CheckValidationLayerSupport(validation_layers)){
        throw std::runtime_error("Validation layers are not supported.");
    }

    is_debug_instance = true;

    using_validation_layers = validation_layers;

    ct_create_info.enabledLayerCount = static_cast<uint32_t>(using_validation_layers.size());
    ct_create_info.ppEnabledLayerNames = using_validation_layers.data();

    printf("Reached Debug Enable\n");
}

std::vector<const char*> CtInstance::GrabExtensions(){
    uint32_t glfw_extension_count = 0;
    const char** glfw_extensions;

    glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

    std::vector<const char*> extensions(glfw_extensions, glfw_extensions + glfw_extension_count);

    if(is_debug_instance){
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        printf("Added VK_EXT_debug_utils to extensions");
    }

    return extensions;
}

void CtInstance::InitializeInstance(CtInstanceApplicationInfo ct_application_info, CtInstanceCreateInfo ct_create_info){

    auto extensions = GrabExtensions();
    
    ct_create_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    ct_create_info.ppEnabledExtensionNames = extensions.data();

    //Let's create our Vulkan stuff
    VkApplicationInfo vk_application_info {};
    vk_application_info.sType = ct_application_info.sType;
    vk_application_info.pNext = ct_application_info.pNext;
    vk_application_info.pApplicationName = ct_application_info.pApplicationName;
    vk_application_info.applicationVersion = ct_application_info.applicationVersion;
    vk_application_info.pEngineName = ct_application_info.pEngineName;
    vk_application_info.engineVersion = ct_application_info.engineVersion;
    vk_application_info.apiVersion = ct_application_info.apiVersion;

    printf("Application name: %s.\n", vk_application_info.pApplicationName);

    VkInstanceCreateInfo vk_create_info {};
    vk_create_info.sType = ct_create_info.sType;
    vk_create_info.pNext = ct_create_info.pNext;
    vk_create_info.pApplicationInfo = &vk_application_info;
    vk_create_info.flags = ct_create_info.flags;
    vk_create_info.enabledLayerCount = ct_create_info.enabledLayerCount;
    vk_create_info.ppEnabledLayerNames = ct_create_info.ppEnabledLayerNames;
    vk_create_info.enabledExtensionCount = ct_create_info.enabledExtensionCount;
    vk_create_info.ppEnabledExtensionNames = ct_create_info.ppEnabledExtensionNames;

    if(is_debug_instance){
        
        CtInstanceDebugUtilsMessengerCreateInfo ct_debug_create_info {}; 
        CreateDebugMessengerInfo(ct_debug_create_info, nullptr, 0, 
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
        DebugCallback, nullptr);

        VkDebugUtilsMessengerCreateInfoEXT vk_debug_create_info {};
        vk_debug_create_info.sType = ct_debug_create_info.sType;
        vk_debug_create_info.messageSeverity = ct_debug_create_info.messageSeverity;
        vk_debug_create_info.messageType = ct_debug_create_info.messageType;
        vk_debug_create_info.pfnUserCallback = ct_debug_create_info.pfnUserCallback;
        
        vk_create_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &vk_debug_create_info;
    }
    
    printf("Enabled layers: %d. Enabled extensions: %d.\n", vk_create_info.enabledLayerCount, vk_create_info.enabledExtensionCount);

    // VkInstance test;
    //Now we can actually do our stuff
    VkResult result = vkCreateInstance(&vk_create_info, nullptr, &instance);

    switch(result){
        case VK_SUCCESS:
            printf("Created Instance Successfully.\n");
            break;
        case VK_ERROR_OUT_OF_HOST_MEMORY:
            throw std::runtime_error("Error in creating instance: Ran out of host memory.");
            break;
        case VK_ERROR_OUT_OF_DEVICE_MEMORY:
            throw std::runtime_error("Error in creating instance: Ran out of device memory.");
            break;
        case VK_ERROR_INITIALIZATION_FAILED:
            throw std::runtime_error("Error in creating instance: Instance initialization failed.");
            break;
        case VK_ERROR_LAYER_NOT_PRESENT:
            throw std::runtime_error("Error in creating instance: A given layer was not present.");
            break;
        case VK_ERROR_EXTENSION_NOT_PRESENT:
            throw std::runtime_error("Error in creating instance: A given extension was not present.");
            break;
        case VK_ERROR_INCOMPATIBLE_DRIVER:
            throw std::runtime_error("Error in creating instance: Driver is incompatible.");
            break;
        default:
            throw std::runtime_error("Unhandled instance creation call.");

    }
}