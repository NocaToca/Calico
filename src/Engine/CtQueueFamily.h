#include <optional>
#include <cstdint>
#include <vulkan/vulkan.h>

struct CtDeviceQueueCreateInfo{

    VkStructureType sType; //Structure type

    const void* pNext; //Feature extension

    VkDeviceQueueCreateFlags flags; //The only flag is VK_DEVICE_QUEUE_CREATE_PROTECTED_BIT, which makes the queue protected-capable

    uint32_t queueFamilyIndex; //The index indicating the queue family in which to create the queues on the device

    uint32_t queueCount; //The amoun of queues on the device

    const float* pQueuePriorities; //The actual queues

};

//I anticipate this class mostly being used by the Device, but I am unsure if it'll expand so it's here as a 
//seperate class. I wanna keep things adaptable!
class CtQueueFamily{

    public:

        bool IsComplete();
        static CtQueueFamily* CreateQueueFamily(VkSurfaceKHR* surface);
        bool TestDevice(VkPhysicalDevice device);
        uint32_t GraphicsFamilyValue(){
            return graphics_family.value();
        }
        uint32_t PresentFamilyValue(){
            return present_family.value();
        }


    private:
        std::optional<uint32_t> graphics_family;
        std::optional<uint32_t> present_family;

        VkQueue graphics_queue;
        VkQueue present_queue;

        VkSurfaceKHR surface;

        void ImplementQueueFamily(VkPhysicalDevice device);
        void PopulateQueueFamilyCreate(CtDeviceQueueCreateInfo& create_info, const void* pNext,
            VkDeviceQueueCreateFlags flags, uint32_t queue_family_index, uint32_t queue_count,
            const float* pointer_to_queue_priorites);
        void TransferQueueFamilyCreate(CtDeviceQueueCreateInfo& ct_create_info, VkDeviceQueueCreateInfo& vk_create_info);

    friend class CtDevice;
    friend class CtRenderer;
};

