#include <vulkan/vulkan.h>
#include <vector>

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


class CtDevice{

    public:

        static CtDevice* CreateDevice(Engine* engine, EngineSettings settings);
    private:
        //The actual GPU
        VkPhysicalDevice physical_device = VK_NULL_HANDLE;
        
        //The interface with the GPU
        VkDevice interface_device;

        //Our Queue Families on our Device
        CtQueueFamily* queue_family;

        //Physical Device
        std::vector<const char*> GetRequiredDeviceExtensions();
        void ChooseDevice(CtInstance ct_instance, CtDeviceRequirments requirements);
        bool IsDeviceSuitable(VkPhysicalDevice device, CtDeviceRequirments requirements);
        float DeviceScore(VkPhysicalDevice device, CtDeviceRequirments requirements); //To be implemented
        static void FillCtDeviceRequirements(CtDeviceRequirments& device_requirements, VkPhysicalDeviceType device_type, bool has_sampler_anisotropy);
        bool CheckDeviceExtensionSupport(VkPhysicalDevice device);

        //Interface Device
        void CreateInterfaceDevice();


    friend class Engine;
    friend class CtQueueFamily;
};