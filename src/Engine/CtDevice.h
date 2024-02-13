#include <vulkan/vulkan.h>
#include <vector>
#include <cstdint>

class CtInstance;
class Engine;
struct EngineSettings;
class CtQueueFamily;

//Basically a set of checks that we can use to check if our device is suitable
struct CtDeviceRequirments{
    //Used for checking to see if the device is a dedicated GPU or not
    VkPhysicalDeviceType device_type;

    //Used to check to see if we want sampler anisotropy
    bool has_sampler_anisotropy;
};

//A struct to basically check our physical device features
struct CtPhysicalDeviceFeatures{
    //Robust buffer accesss entails a lot, but as a summary, it makes sure that any buffer checks perform within the bounds of the buffer
    VkBool32 robustBufferAccess;
    //Enables full 32-bit range for indices. 
    VkBool32 fullDrawIndexUint32;
    //Specifies whether image view arrays can be created - things like SampledCubeArray and ImageCubeArray
    VkBool32 imageCubeArray;
    //Specifies if the VkPipelineColorBlendAttachmetState (color blending) is controlled independently or the same for all shaders
    VkBool32 independentBlend;
    //Enabled geometry shaders
    VkBool32 geometryShader;
    //Enables tessellation shaders and control
    VkBool32 tessellationShader;
    //Enables sample shading support and multisample interpolation
    VkBool32 sampleRateShading;
    //Whether blending operations take two sources or not
    VkBool32 dualSrcBlend;
    VkBool32 logicOp;
    VkBool32 multiDrawIndirect;
    VkBool32 drawIndirectFirstInstance;
    VkBool32 depthClamp;
    VkBool32 depthBiasClamp;
    VkBool32 fillModeNonSolid;
    VkBool32 depthBounds;
    VkBool32 wideLines;
    VkBool32 largePoints;
    VkBool32 alphaToOne;
    VkBool32 multiViewport;
    VkBool32 samplerAnisotropy;
    VkBool32 textureCompressionETC2;
    VkBool32 textureCompressionASTC_LDR;
    VkBool32 textureCompressionBC;
    VkBool32 occlusionQueryPrecise;
    VkBool32 pipelineStatisticsQuery;
    VkBool32 vertexPipelineStoresAndAtomics;
    VkBool32 fragmentStoresAndAtomics;
    VkBool32 shaderTessellationAndGeometryPointSize;
    VkBool32 shaderImageGatherExtended;
    VkBool32 shaderStorageImageExtendedFormats;
    VkBool32 shaderStorageImageMultisample;
    VkBool32 shaderStorageImageReadWithoutFormat;
    VkBool32 shaderStorageImageWriteWithoutFormat;
    VkBool32 shaderUniformBufferArrayDynamicIndexing;
    VkBool32 shaderSampledImageArrayDynamicIndexing;
    VkBool32 shaderStorageBufferArrayDynamicIndexing;
    VkBool32 shaderStorageImageArrayDynamicIndexing;
    VkBool32 shaderClipDistance;
    VkBool32 shaderCullDistance;
    VkBool32 shaderFloat64;
    VkBool32 shaderInt64;
    VkBool32 shaderInt16;
    VkBool32 shaderResourceResidency;
    VkBool32 shaderResourceMinLod;
    VkBool32 sparseResidencyBuffer;
    VkBool32 sparseResidencyImage2D;
    VkBool32 sparseResidencyImage3D;
    VkBool32 sparseResidency2Samples;
    VkBool32 sparseResidency4Samples;
    VkBool32 sparseResidency8Samples;
    VkBool32 sparseResidency16Samples;
    VkBool32 sparseResidencyAliased;
    VkBool32 variableMultisampleRate;
    VkBool32 inheritedQueries;
};

enum CtPhysicalDeviceFeatureEnable {
    ROBUST_BUFFER_ACCESS_ENABLE,
    FULL_DRAW_INDEX_UINT32_ENABLE,
    IMAGE_CUBE_ARRAY_ENABLE,
    INDEPENDENT_BLEND_ENABLE,
    GEOMETRY_SHADER_ENABLE,
    TESSELLATION_SHADER_ENABLE,
    SAMPLE_RATE_SHADING_ENABLE,
    DUAL_SRC_BLEND_ENABLE,
    LOGIC_OP_ENABLE,
    MULTI_DRAW_INDIRECT_ENABLE,
    DRAW_INDIRECT_FIRST_INSTANCE_ENABLE,
    DEPTH_CLAMP_ENABLE,
    DEPTH_BIAS_CLAMP_ENABLE,
    FILL_MODE_NON_SOLID_ENABLE,
    DEPTH_BOUNDS_ENABLE,
    WIDE_LINES_ENABLE,
    LARGE_POINTS_ENABLE,
    ALPHA_TO_ONE_ENABLE,
    MULTI_VIEWPORT_ENABLE,
    SAMPLER_ANISOTROPY_ENABLE,
    TEXTURE_COMPRESSION_ETC2_ENABLE,
    TEXTURE_COMPRESSION_ASTC_LDR_ENABLE,
    TEXTURE_COMPRESSION_BC_ENABLE,
    OCCLUSION_QUERY_PRECISE_ENABLE,
    PIPELINE_STATISTICS_QUERY_ENABLE,
    VERTEX_PIPELINE_STORES_AND_ATOMICS_ENABLE,
    FRAGMENT_STORES_AND_ATOMICS_ENABLE,
    SHADER_TESSELLATION_AND_GEOMETRY_POINT_SIZE_ENABLE,
    SHADER_IMAGE_GATHER_EXTENDED_ENABLE,
    SHADER_STORAGE_IMAGE_EXTENDED_FORMATS_ENABLE,
    SHADER_STORAGE_IMAGE_MULTISAMPLE_ENABLE,
    SHADER_STORAGE_IMAGE_READ_WITHOUT_FORMAT_ENABLE,
    SHADER_STORAGE_IMAGE_WRITE_WITHOUT_FORMAT_ENABLE,
    SHADER_UNIFORM_BUFFER_ARRAY_DYNAMIC_INDEXING_ENABLE,
    SHADER_SAMPLED_IMAGE_ARRAY_DYNAMIC_INDEXING_ENABLE,
    SHADER_STORAGE_BUFFER_ARRAY_DYNAMIC_INDEXING_ENABLE,
    SHADER_STORAGE_IMAGE_ARRAY_DYNAMIC_INDEXING_ENABLE,
    SHADER_CLIP_DISTANCE_ENABLE,
    SHADER_CULL_DISTANCE_ENABLE,
    SHADER_FLOAT64_ENABLE,
    SHADER_INT64_ENABLE,
    SHADER_INT16_ENABLE,
    SHADER_RESOURCE_RESIDENCY_ENABLE,
    SHADER_RESOURCE_MIN_LOD_ENABLE,
    SPARSE_RESIDENCY_BUFFER_ENABLE,
    SPARSE_RESIDENCY_IMAGE_2D_ENABLE,
    SPARSE_RESIDENCY_IMAGE_3D_ENABLE,
    SPARSE_RESIDENCY_2_SAMPLES_ENABLE,
    SPARSE_RESIDENCY_4_SAMPLES_ENABLE,
    SPARSE_RESIDENCY_8_SAMPLES_ENABLE,
    SPARSE_RESIDENCY_16_SAMPLES_ENABLE,
    SPARSE_RESIDENCY_ALIASED_ENABLE,
    VARIABLE_MULTISAMPLE_RATE_ENABLE,
    INHERITED_QUERIES_ENABLE
};

struct CtInterfaceDeviceCreateInfo{

    //The structure type of this device 
    VkStructureType sType;

    const void* pNext; //The structure extension

    VkDeviceCreateFlags flags; //Currently no use, as there are no flags we can currently use on our interface creation

    uint32_t queueCreateInfoCount; //The size of our queue creation

    const VkDeviceQueueCreateInfo* pQueueCreateInfos; //The array of our queue creations

    uint32_t enabledLayerCount; //The size of our enabled layers

    const char* const* ppEnabledLayerNames; //The names of our enabled layers

    uint32_t enabledExtensionCount; //The number of extensions we have

    const char* const* ppEnabledExtensionNames; //The names of our extensions

    const VkPhysicalDeviceFeatures* pEnabledFeatures; //Our enabled features on the device

};

class CtDevice{

    public:

        static CtDevice* CreateDevice(Engine* engine, EngineSettings settings);
        VkDevice* GetInterfaceDevice();
        VkPhysicalDevice* GetPhysicalDevice(){
            return &physical_device;
        }

    private:
        //The actual GPU
        VkPhysicalDevice physical_device = VK_NULL_HANDLE;
        
        //The interface with the GPU
        VkDevice interface_device;

        //Our Queue Families on our Device
        CtQueueFamily* queue_family;

        //Our enabled features
        CtPhysicalDeviceFeatures features;

        //Enabling a feature
        void EnableFeature(CtPhysicalDeviceFeatures& feature, CtPhysicalDeviceFeatureEnable enable);
        void TransferFeatures(CtPhysicalDeviceFeatures& device_features, VkPhysicalDeviceFeatures& features);

        //Physical Device
        std::vector<const char*> GetRequiredDeviceExtensions();
        void ChooseDevice(CtInstance ct_instance, CtDeviceRequirments requirements);
        bool IsDeviceSuitable(VkPhysicalDevice device, CtDeviceRequirments requirements);
        float DeviceScore(VkPhysicalDevice device, CtDeviceRequirments requirements); //To be implemented
        static void FillCtDeviceRequirements(CtDeviceRequirments& device_requirements, VkPhysicalDeviceType device_type, bool has_sampler_anisotropy);
        bool CheckDeviceExtensionSupport(VkPhysicalDevice device);

        //Interface Device
        void PopulateCreateInfo(CtInterfaceDeviceCreateInfo& create_info,
            const void* pointer_to_next, VkDeviceCreateFlags flags, uint32_t queue_create_info_count,
            const VkDeviceQueueCreateInfo* pointer_to_queue_create_infos, uint32_t enabled_layer_count,
            const char* const* pointer_to_pointer_of_enabled_layer_names, uint32_t enabled_extensions_count,
            const char* const* pointer_to_pointer_of_enabled_extension_names,
            const VkPhysicalDeviceFeatures* pointer_to_enabled_features);
        void TransferCreateInfo(CtInterfaceDeviceCreateInfo& ct_create_info, VkDeviceCreateInfo& vk_create_info);
        void CreateInterfaceDevice();


    friend class Engine;
    friend class CtQueueFamily;
    friend class CtSwapchain;
};