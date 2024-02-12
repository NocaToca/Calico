#include <optional>
#include <cstdint>
#include <vulkan/vulkan.h>

//I anticipate this class mostly being used by the Device, but I am unsure if it'll expand so it's here as a 
//seperate class. I wanna keep things adaptable!
class CtQueueFamily{

    public:

        bool IsComplete();
        static CtQueueFamily* CreateQueueFamily(VkSurfaceKHR* surface);
        bool TestDevice(VkPhysicalDevice device);


    private:
        std::optional<uint32_t> graphics_family;
        std::optional<uint32_t> present_family;

        VkSurfaceKHR surface;

        void ImplementQueueFamily(VkPhysicalDevice device);

    friend class CtDevice;
};

