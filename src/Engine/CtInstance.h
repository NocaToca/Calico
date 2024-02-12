
#include <cstdint>
#include <vulkan/vulkan.h>
#include <vector>
#include <iostream>
#include <memory>

//Forward declaring everything so I know the types of each
/**
 * A big thing for me is to understand the API I'm interacting with. To do this, I am essentially recreating everything so I know
 * what everything does. This is understandably inneficent, but the purpose of this project is to learn how engines work
 * 
 * It wont be an amazingly efficient engine and I don't intend for it to compete with the big bois in anyway :3 
*/
struct CtInstanceApplicationInfo{
    VkStructureType sType; //The structure type of this instance

    const void* pNext; //Default to NULL, but could point to a structure that extends this structure

    const char* pApplicationName; //Quite simply the name of the application (can be null)

    uint32_t applicationVersion; //The name of the version

    const char* pEngineName; //The name of the engine used to create the application (This is esentially the application for us)

    uint32_t engineVersion; //The version of our engine

    uint32_t apiVersion; //The Vulkan api version

};

//The information to actually create our instance
struct CtInstanceCreateInfo{
    VkStructureType sType; //The structure type

    const void* pNext; //Default to NULL, but could point to an extending structure

    VkInstanceCreateFlags flags; //The creation flags of the instance create flags.
    //The only value of vlags that I have found is VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR which basically just says that the 
    //instance will go across Vulkan Portability compliant devices and groups in addition to the normal devices
    const VkApplicationInfo* pApplicationInfo; //The application information. We generated that from the above structure

    uint32_t enabledLayerCount; //Kinda self explanitory, but the number of global layers that are enabled (or, more accurately, to enable)

    const char* const* ppEnabledLayerNames; //The names of the layers to enable. Interestingly, the order loaded matters, as the first in this list will be closest
    //to the application while the last will be closest to the driver

    uint32_t enabledExtensionCount; //The number of extensions to enable

    const char* const* ppEnabledExtensionNames; //The same idea as enabled layer names, but for exensions.

};

//The information for out debug utils
struct CtInstanceDebugUtilsMessengerCreateInfo{
    VkStructureType sType; //The structure type

    const void* pNext; //The structure extension

    VkDebugUtilsMessengerCreateFlagsEXT flags; //Flags currently has no function, so it should always be 0

    VkDebugUtilsMessageSeverityFlagsEXT messageSeverity; //Specifies which severity of events will call this function. These values can be:
    /**
     * VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT - Outputs all diagnostic messages from the Vulkan loader, layers, and drivers
     * VK_DEBUG_UTILS_MESSAGE_SEVERTIY_INFO_BIT_EXT - Outputs informational outputs, like resource details
     * VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT - Outputs warnings that may be app bugs
     * VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT - Errors!
    */

    VkDebugUtilsMessageTypeFlagsEXT messageType; //Specifies which events will actually callback. These values can be:
    /**
     * VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT - General events. Mostly just not a specification and not a performance event
     * VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT - Specifies something that has occurred during validation against Vulkan that may be bad behavoir! You might be using Vulkan wrong
     * VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT - Specifies a non-optimal use of Vulkan
     * VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT - Specifies that GPU addresses have been modified
    */

    PFN_vkDebugUtilsMessengerCallbackEXT pfnUserCallback; //The callback function to trigger

    void* pUserData; //The user data passed to the call back
};


class CtInstance{

    public:
        CtInstance();
        ~CtInstance();

        //I don't really like the structure type idea of Vulkan, so I'm going to embed it in functions
        //It also forces me to understand and complete each variable
        void CreateApplicationInfo(CtInstanceApplicationInfo& app_info, const void* pointer_next,
            const char* pointer_application_name,
            uint32_t application_version,
            const char* pointer_engine_name,
            uint32_t engine_version,
            uint32_t api_version);

        void CreateInstanceInfo(CtInstanceCreateInfo& create_info, const void* pointer_next,
            VkInstanceCreateFlags flags,
            uint32_t enabled_layer_count,
            const char* const* pointer_of_pointer_enabled_layer_names,
            uint32_t enabled_extension_count,
            const char* const* pointer_of_pointer_enabled_extension_names);

        std::vector<const char*> GrabExtensions();

        void EnableDebugInstance(CtInstanceCreateInfo& ct_create_info, std::vector<const char*> validation_layers);

        void SetupDebugInstance();

        void InitializeInstance(CtInstanceApplicationInfo ct_application_info, CtInstanceCreateInfo ct_create_info);

        VkInstance* GetInstance(){
            return &instance;
        }

    private:
        VkInstance instance;

        VkDebugUtilsMessengerEXT debug_messenger;

        std::vector<const char*> using_validation_layers;

        bool is_debug_instance = false;

        bool CheckValidationLayerSupport(std::vector<const char*> validation_layers);

        void CreateDebugMessengerInfo(CtInstanceDebugUtilsMessengerCreateInfo& debug_info, const void* pointer_next,
            VkDebugUtilsMessengerCreateFlagsEXT flags,
            VkDebugUtilsMessageSeverityFlagsEXT message_severity,
            VkDebugUtilsMessageTypeFlagsEXT message_type,
            PFN_vkDebugUtilsMessengerCallbackEXT pointer_to_function_user_callback,
            void* user_data);

        static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT message_servity,
            VkDebugUtilsMessageTypeFlagsEXT message_type,
            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
            void* p_user_data
        ){
            std::cerr << "Validation layer: " << pCallbackData -> pMessage << std::endl;

            return VK_FALSE;
        }

        VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                VkDebugUtilsMessengerEXT* pDebugMessenger);
};