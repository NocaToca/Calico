#include "CtDevice.h"
#include "CtInstance.h"
#include <cstdint>
#include <vector>
#include "Engine.h"
#include <set>
#include <string>
#include "CtQueueFamily.h"
#include "CtWindow.h"
#include "CtSwapchain.h"

CtDevice* CtDevice::CreateDevice(Engine* ct_engine, EngineSettings settings){
    CtDevice* ct_device = new CtDevice();

    CtDeviceRequirments requirements {};
    FillCtDeviceRequirements(requirements, VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU, true);

    ct_device->queue_family = CtQueueFamily::CreateQueueFamily(ct_engine->window->GetSurface());

    //Physical Device phase
    ct_device->ChooseDevice(*(ct_engine->instance), requirements);
    ct_device->queue_family->ImplementQueueFamily(ct_device->physical_device);
    ct_device->CreateInterfaceDevice();

    return ct_device;

}

/******************************PHYSICAL DEVICE*******************************/

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

    bool queue_families_supported = queue_family->TestDevice(device);

    bool swap_chain_support = false;
    if(extensions_supported){
        CtSwapchainSupportDetails support_details = CtSwapchain::QuerySwapchainSupport(device, &queue_family->surface); 
    }

    return  device_properties.deviceType == requirements.device_type &&
            device_features.samplerAnisotropy == requirements.has_sampler_anisotropy &&
            extensions_supported &&
            queue_families_supported;

}

void CtDevice::ChooseDevice(CtInstance ct_instance, CtDeviceRequirments requirements){

    //Let's go through and choose a GPU
    uint32_t device_count = 0;

    if(&ct_instance == nullptr){
        throw std::runtime_error("Instance deleted itself.");
    }

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

    // printf("Checkpoint");

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

/**************************************************INTERFACE DEVICE**********************************************************/

void CtDevice::PopulateCreateInfo(CtInterfaceDeviceCreateInfo& create_info,
            const void* pointer_to_next, VkDeviceCreateFlags flags, uint32_t queue_create_info_count,
            const VkDeviceQueueCreateInfo* pointer_to_queue_create_infos, uint32_t enabled_layer_count,
            const char* const* pointer_to_pointer_of_enabled_layer_names, uint32_t enabled_extensions_count,
            const char* const* pointer_to_pointer_of_enabled_extension_names,
            const VkPhysicalDeviceFeatures* pointer_to_enabled_features){

    create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    create_info.pNext = pointer_to_next;
    create_info.flags = flags;
    create_info.queueCreateInfoCount = queue_create_info_count;
    create_info.pQueueCreateInfos = pointer_to_queue_create_infos;
    create_info.enabledLayerCount = enabled_layer_count;
    create_info.ppEnabledLayerNames = pointer_to_pointer_of_enabled_layer_names;
    create_info.enabledExtensionCount = enabled_extensions_count;
    create_info.ppEnabledExtensionNames = pointer_to_pointer_of_enabled_extension_names;
    create_info.pEnabledFeatures = pointer_to_enabled_features;
}

void CtDevice::TransferCreateInfo(CtInterfaceDeviceCreateInfo& ct_create_info, VkDeviceCreateInfo& vk_create_info){
    vk_create_info.sType = ct_create_info.sType;
    vk_create_info.pNext = ct_create_info.pNext;
    vk_create_info.flags = ct_create_info.flags;
    vk_create_info.queueCreateInfoCount = ct_create_info.queueCreateInfoCount;
    vk_create_info.pQueueCreateInfos = ct_create_info.pQueueCreateInfos;
    vk_create_info.enabledLayerCount = ct_create_info.enabledLayerCount;
    vk_create_info.ppEnabledLayerNames = ct_create_info.ppEnabledLayerNames;
    vk_create_info.enabledExtensionCount = ct_create_info.enabledExtensionCount;
    vk_create_info.ppEnabledExtensionNames = ct_create_info.ppEnabledExtensionNames;
    vk_create_info.pEnabledFeatures = ct_create_info.pEnabledFeatures;
}

//Actually responsible for creating our interface device
void CtDevice::CreateInterfaceDevice(){

    std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
    std::set<uint32_t> unique_queue_families = {queue_family->graphics_family.value(), queue_family->present_family.value()};

    float queue_priority = 1.0f;
    for(uint32_t queue_family_index : unique_queue_families){
        CtDeviceQueueCreateInfo ct_queue_create_info {};
        queue_family->PopulateQueueFamilyCreate(ct_queue_create_info, nullptr, 0, queue_family_index, 1, &queue_priority);
        
        VkDeviceQueueCreateInfo vk_queue_create_info {};
        queue_family->TransferQueueFamilyCreate(ct_queue_create_info, vk_queue_create_info);

        queue_create_infos.push_back(vk_queue_create_info);
    }

    CtPhysicalDeviceFeatures ct_device_features {};
    EnableFeature(ct_device_features, SAMPLER_ANISOTROPY_ENABLE);

    VkPhysicalDeviceFeatures vk_device_features {};
    TransferFeatures(ct_device_features, vk_device_features);

    std::vector<const char*> extensions = GetRequiredDeviceExtensions();

    CtInterfaceDeviceCreateInfo ct_interface_create_info {};
    PopulateCreateInfo(ct_interface_create_info, nullptr, 0, 
        static_cast<uint32_t>(queue_create_infos.size()), queue_create_infos.data(),
        0, nullptr,
        static_cast<uint32_t>(extensions.size()), extensions.data(),
        &vk_device_features);
    
    VkDeviceCreateInfo vk_interface_create_info {};
    TransferCreateInfo(ct_interface_create_info, vk_interface_create_info);

    if(vkCreateDevice(physical_device, &vk_interface_create_info, nullptr, &interface_device) != VK_SUCCESS){
        throw std::runtime_error("Failed to create an interface device");
    }

    printf("Created interface device.\n");

    vkGetDeviceQueue(interface_device, queue_family->graphics_family.value(), 0, &(queue_family->graphics_queue));
    vkGetDeviceQueue(interface_device, queue_family->present_family.value(), 0, &(queue_family->present_queue));

}

VkDevice* CtDevice::GetInterfaceDevice(){
    return &interface_device;
}

/******************************************************FEATURES ENABLE**********************************************************************/

void CtDevice::TransferFeatures(CtPhysicalDeviceFeatures& device_features, VkPhysicalDeviceFeatures& features){
    device_features.robustBufferAccess = features.robustBufferAccess;
    device_features.fullDrawIndexUint32 = features.fullDrawIndexUint32;
    device_features.imageCubeArray = features.imageCubeArray;
    device_features.independentBlend = features.independentBlend;
    device_features.geometryShader = features.geometryShader;
    device_features.tessellationShader = features.tessellationShader;
    device_features.sampleRateShading = features.sampleRateShading;
    device_features.dualSrcBlend = features.dualSrcBlend;
    device_features.logicOp = features.logicOp;
    device_features.multiDrawIndirect = features.multiDrawIndirect;
    device_features.drawIndirectFirstInstance = features.drawIndirectFirstInstance;
    device_features.depthClamp = features.depthClamp;
    device_features.depthBiasClamp = features.depthBiasClamp;
    device_features.fillModeNonSolid = features.fillModeNonSolid;
    device_features.depthBounds = features.depthBounds;
    device_features.wideLines = features.wideLines;
    device_features.largePoints = features.largePoints;
    device_features.alphaToOne = features.alphaToOne;
    device_features.multiViewport = features.multiViewport;
    device_features.samplerAnisotropy = features.samplerAnisotropy;
    device_features.textureCompressionETC2 = features.textureCompressionETC2;
    device_features.textureCompressionASTC_LDR = features.textureCompressionASTC_LDR;
    device_features.textureCompressionBC = features.textureCompressionBC;
    device_features.occlusionQueryPrecise = features.occlusionQueryPrecise;
    device_features.pipelineStatisticsQuery = features.pipelineStatisticsQuery;
    device_features.vertexPipelineStoresAndAtomics = features.vertexPipelineStoresAndAtomics;
    device_features.fragmentStoresAndAtomics = features.fragmentStoresAndAtomics;
    device_features.shaderTessellationAndGeometryPointSize = features.shaderTessellationAndGeometryPointSize;
    device_features.shaderImageGatherExtended = features.shaderImageGatherExtended;
    device_features.shaderStorageImageExtendedFormats = features.shaderStorageImageExtendedFormats;
    device_features.shaderStorageImageMultisample = features.shaderStorageImageMultisample;
    device_features.shaderStorageImageReadWithoutFormat = features.shaderStorageImageReadWithoutFormat;
    device_features.shaderStorageImageWriteWithoutFormat = features.shaderStorageImageWriteWithoutFormat;
    device_features.shaderUniformBufferArrayDynamicIndexing = features.shaderUniformBufferArrayDynamicIndexing;
    device_features.shaderSampledImageArrayDynamicIndexing = features.shaderSampledImageArrayDynamicIndexing;
    device_features.shaderStorageBufferArrayDynamicIndexing = features.shaderStorageBufferArrayDynamicIndexing;
    device_features.shaderStorageImageArrayDynamicIndexing = features.shaderStorageImageArrayDynamicIndexing;
    device_features.shaderClipDistance = features.shaderClipDistance;
    device_features.shaderCullDistance = features.shaderCullDistance;
    device_features.shaderFloat64 = features.shaderFloat64;
    device_features.shaderInt64 = features.shaderInt64;
    device_features.shaderInt16 = features.shaderInt16;
    device_features.shaderResourceResidency = features.shaderResourceResidency;
    device_features.shaderResourceMinLod = features.shaderResourceMinLod;
    device_features.sparseResidencyBuffer = features.sparseResidencyBuffer;
    device_features.sparseResidencyImage2D = features.sparseResidencyImage2D;
    device_features.sparseResidencyImage3D = features.sparseResidencyImage3D;
    device_features.sparseResidency2Samples = features.sparseResidency2Samples;
    device_features.sparseResidency4Samples = features.sparseResidency4Samples;
    device_features.sparseResidency8Samples = features.sparseResidency8Samples;
    device_features.sparseResidency16Samples = features.sparseResidency16Samples;
    device_features.sparseResidencyAliased = features.sparseResidencyAliased;
    device_features.variableMultisampleRate = features.variableMultisampleRate;
    device_features.inheritedQueries = features.inheritedQueries;
}

void CtDevice::EnableFeature(CtPhysicalDeviceFeatures& feature, CtPhysicalDeviceFeatureEnable enable){
    switch(enable){
        case ROBUST_BUFFER_ACCESS_ENABLE:
            feature.robustBufferAccess = VK_TRUE;
            return;
        case FULL_DRAW_INDEX_UINT32_ENABLE:
            feature.fullDrawIndexUint32 = VK_TRUE;
            return;
        case IMAGE_CUBE_ARRAY_ENABLE:
            feature.imageCubeArray = VK_TRUE;
            return;
        case INDEPENDENT_BLEND_ENABLE:
            feature.independentBlend = VK_TRUE;
            return;
        case GEOMETRY_SHADER_ENABLE:
            feature.geometryShader = VK_TRUE;
            return;
        case TESSELLATION_SHADER_ENABLE:
            feature.tessellationShader = VK_TRUE;
            return;
        case SAMPLE_RATE_SHADING_ENABLE:
            feature.sampleRateShading = VK_TRUE;
            return;
        case DUAL_SRC_BLEND_ENABLE:
            feature.dualSrcBlend = VK_TRUE;
            return;
        case LOGIC_OP_ENABLE:
            feature.logicOp = VK_TRUE;
            return;
        case MULTI_DRAW_INDIRECT_ENABLE:
            feature.multiDrawIndirect = VK_TRUE;
            return;
        case DRAW_INDIRECT_FIRST_INSTANCE_ENABLE:
            feature.drawIndirectFirstInstance = VK_TRUE;
            return;
        case DEPTH_CLAMP_ENABLE:
            feature.depthClamp = VK_TRUE;
            return;
        case DEPTH_BIAS_CLAMP_ENABLE:
            feature.depthBiasClamp = VK_TRUE;
            return;
        case FILL_MODE_NON_SOLID_ENABLE:
            feature.fillModeNonSolid = VK_TRUE;
            return;
        case DEPTH_BOUNDS_ENABLE:
            feature.depthBounds = VK_TRUE;
            return;
        case WIDE_LINES_ENABLE:
            feature.wideLines = VK_TRUE;
            return;
        case LARGE_POINTS_ENABLE:
            feature.largePoints = VK_TRUE;
            return;
        case ALPHA_TO_ONE_ENABLE:
            feature.alphaToOne = VK_TRUE;
            return;
        case MULTI_VIEWPORT_ENABLE:
            feature.multiViewport = VK_TRUE;
            return;
        case SAMPLER_ANISOTROPY_ENABLE:
            feature.samplerAnisotropy = VK_TRUE;
            return;
        case TEXTURE_COMPRESSION_ETC2_ENABLE:
            feature.textureCompressionETC2 = VK_TRUE;
            return;
        case TEXTURE_COMPRESSION_ASTC_LDR_ENABLE:
            feature.textureCompressionASTC_LDR = VK_TRUE;
            return;
        case TEXTURE_COMPRESSION_BC_ENABLE:
            feature.textureCompressionBC = VK_TRUE;
            return;
        case OCCLUSION_QUERY_PRECISE_ENABLE:
            feature.occlusionQueryPrecise = VK_TRUE;
            return;
        case PIPELINE_STATISTICS_QUERY_ENABLE:
            feature.pipelineStatisticsQuery = VK_TRUE;
            return;
        case VERTEX_PIPELINE_STORES_AND_ATOMICS_ENABLE:
            feature.vertexPipelineStoresAndAtomics = VK_TRUE;
            return;
        case FRAGMENT_STORES_AND_ATOMICS_ENABLE:
            feature.fragmentStoresAndAtomics = VK_TRUE;
            return;
        case SHADER_TESSELLATION_AND_GEOMETRY_POINT_SIZE_ENABLE:
            feature.shaderTessellationAndGeometryPointSize = VK_TRUE;
            return;
        case SHADER_IMAGE_GATHER_EXTENDED_ENABLE:
            feature.shaderImageGatherExtended = VK_TRUE;
            return;
        case SHADER_STORAGE_IMAGE_EXTENDED_FORMATS_ENABLE:
            feature.shaderStorageImageExtendedFormats = VK_TRUE;
            return;
        case SHADER_STORAGE_IMAGE_MULTISAMPLE_ENABLE:
            feature.shaderStorageImageMultisample = VK_TRUE;
            return;
        case SHADER_STORAGE_IMAGE_READ_WITHOUT_FORMAT_ENABLE:
            feature.shaderStorageImageReadWithoutFormat = VK_TRUE;
            return;
        case SHADER_STORAGE_IMAGE_WRITE_WITHOUT_FORMAT_ENABLE:
            feature.shaderStorageImageWriteWithoutFormat = VK_TRUE;
            return;
        case SHADER_UNIFORM_BUFFER_ARRAY_DYNAMIC_INDEXING_ENABLE:
            feature.shaderUniformBufferArrayDynamicIndexing = VK_TRUE;
            return;
        case SHADER_SAMPLED_IMAGE_ARRAY_DYNAMIC_INDEXING_ENABLE:
            feature.shaderSampledImageArrayDynamicIndexing = VK_TRUE;
            return;
        case SHADER_STORAGE_BUFFER_ARRAY_DYNAMIC_INDEXING_ENABLE:
            feature.shaderStorageBufferArrayDynamicIndexing = VK_TRUE;
            return;
        case SHADER_STORAGE_IMAGE_ARRAY_DYNAMIC_INDEXING_ENABLE:
            feature.shaderStorageImageArrayDynamicIndexing = VK_TRUE;
            return;
        case SHADER_CLIP_DISTANCE_ENABLE:
            feature.shaderClipDistance = VK_TRUE;
            return;
        case SHADER_CULL_DISTANCE_ENABLE:
            feature.shaderCullDistance = VK_TRUE;
            return;
        case SHADER_FLOAT64_ENABLE:
            feature.shaderFloat64 = VK_TRUE;
            return;
        case SHADER_INT64_ENABLE:
            feature.shaderInt64 = VK_TRUE;
            return;
        case SHADER_INT16_ENABLE:
            feature.shaderInt16 = VK_TRUE;
            return;
        case SHADER_RESOURCE_RESIDENCY_ENABLE:
            feature.shaderResourceResidency = VK_TRUE;
            return;
        case SHADER_RESOURCE_MIN_LOD_ENABLE:
            feature.shaderResourceMinLod = VK_TRUE;
            return;
        case SPARSE_RESIDENCY_BUFFER_ENABLE:
            feature.sparseResidencyBuffer = VK_TRUE;
            return;
        case SPARSE_RESIDENCY_IMAGE_2D_ENABLE:
            feature.sparseResidencyImage2D = VK_TRUE;
            return;
        case SPARSE_RESIDENCY_IMAGE_3D_ENABLE:
            feature.sparseResidencyImage3D = VK_TRUE;
            return;
        case SPARSE_RESIDENCY_2_SAMPLES_ENABLE:
            feature.sparseResidency2Samples = VK_TRUE;
            return;
        case SPARSE_RESIDENCY_4_SAMPLES_ENABLE:
            feature.sparseResidency4Samples = VK_TRUE;
            return;
        case SPARSE_RESIDENCY_8_SAMPLES_ENABLE:
            feature.sparseResidency8Samples = VK_TRUE;
            return;
        case SPARSE_RESIDENCY_16_SAMPLES_ENABLE:
            feature.sparseResidency16Samples = VK_TRUE;
            return;
        case SPARSE_RESIDENCY_ALIASED_ENABLE:
            feature.sparseResidencyAliased = VK_TRUE;
            return;
        case VARIABLE_MULTISAMPLE_RATE_ENABLE:
            feature.variableMultisampleRate = VK_TRUE;
            return;
        case INHERITED_QUERIES_ENABLE:
            feature.inheritedQueries = VK_TRUE;
            return;
        default:
            throw std::runtime_error("Unknown feature");
    }
}