#include <vulkan/vulkan.h>
#include <vector>

class CtDevice;
class Engine;
class CtWindow;
class CtGraphicsPipeline;

struct CtSwapchainSupportDetails{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> present_modes;
};

struct CtSwapchainCreateInfoKHR{
    
    //The structure type of this creation information
    VkStructureType                  sType;

    //The pointer to the structure extension
    const void*                      pNext;

    //The flags of the swap chain
    VkSwapchainCreateFlagsKHR        flags; 
    /*
        The options for the flag are as follows:
        VK_SWAPCHAIN_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT_KHR //Specifies that images created from the swap chain must use VK_IMAGE_CREATE_SPIT_INSTANCE_BIND_REGIONS_BIT
        VK_SWAPCHAIN_CREATE_PROTECTED_BIT_KHR                   //Specifies that images created from this swapchain are protected images
        VK_SWAPCHAIN_CREATE_MUTABLE_FORMAT_BIT_KHR              //Specifies that images can be used to create an image view with a different format than the image itself was created bit
        VK_SWAPCHAIN_CREATE_DEFERRED_MEMORY_ALLOCATION_BIT_EXT  //Specifies that the swap chain may defer allocation of memory associated with each swapchain image.
    */
    //The surface that our swapchain will wrtie to
    VkSurfaceKHR                     surface;

    //The minimum of images that the application needs
    uint32_t                         minImageCount;

    //The format that the images will be created with
    VkFormat                         imageFormat;

    //The color space the images will be created with
    VkColorSpaceKHR                  imageColorSpace;

    //The size of the images that the swapchain will create
    VkExtent2D                       imageExtent;

    //The number of views for stereoscopic-3D applications - this should always be one for now in Calico
    uint32_t                         imageArrayLayers;

    //Flags that describe the intended usage of the swapchain images.
    VkImageUsageFlags                imageUsage; //These can be seen here: https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkImageUsageFlagBits.html
    //However, I have a decent idea of what each flag means and I don't want to populate this struct too much with review information

    //Allows images to be shared within swapchains
    VkSharingMode                    imageSharingMode;

    //The amount of queue families that will have access to the swapchian
    uint32_t                         queueFamilyIndexCount;

    //The pointer to the aforementioned queue families
    const uint32_t*                  pQueueFamilyIndices;

    //Describes the default transform of images within the engine
    VkSurfaceTransformFlagBitsKHR    preTransform;

    //Flags for describing how the alpha channel will work when interacting with multiple images
    VkCompositeAlphaFlagBitsKHR      compositeAlpha;

    //The presentation mode the swapchain will use
    VkPresentModeKHR                 presentMode;

    //Whether or not Vulkan is allowed to discard rendering that will not be visible
    VkBool32                         clipped;
    
    //The previous swapchain that was associated with surface
    VkSwapchainKHR                   oldSwapchain;
};

struct CtImageViewCreateInfo{
    //The structure type
    VkStructureType            sType;

    //The pointer to the structure extension
    const void*                pNext;

    //Our image creation flags
    VkImageViewCreateFlags     flags;
    /*
        These values can be:
        VK_IMAGE_VIEW_CREATE_FRAGMENT_DENSITY_MAP_DYNAMIC_BIT_EXT       //Specifies that the density map will be read by the device
        VK_IMAGE_VIEW_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT   //Specifies that the fragment density map will be read by the host
        VK_IMAGE_VIEW_CREATE_FRAGMENT_DENSITY_MAP_DEFERRED_BIT_EXT      //Specifies that the image view can be used with descriptor buffers when capturing and replaying
    */

    //The image that the image view will be create for
    VkImage                    image;

    //The type of the new image
    VkImageViewType            viewType;

    //The format specifying the format and type 
    VkFormat                   format;

    //The component mapping structure for color
    VkComponentMapping         components;

    //Selects the set of mipmap levels and array levels to be accessible to the view
    VkImageSubresourceRange    subresourceRange;
};

class CtSwapchain{

    public:
        static CtSwapchainSupportDetails QuerySwapchainSupport(VkPhysicalDevice physical_device, VkSurfaceKHR* surface);
        static CtSwapchain* CreateSwapchain(Engine* ct_engine);
        static VkImageView CreateImageView(CtDevice* device, VkImage image, VkFormat format, VkImageAspectFlags aspect_flags);
        
        void RecreateSwapchain();

        VkExtent2D GetSwapchainExtent(){
            return swapchain_extent;
        }

    private:

        //The device that this swapchain belongs to
        CtDevice* device;

        CtWindow* window;

        VkSwapchainKHR swapchain;
        std::vector<VkImage> swapchain_images;
        VkFormat swapchain_image_format;
        VkExtent2D swapchain_extent;
        std::vector<VkImageView> swapchain_image_views;

        void PopulateSwapchainCreateInfo(CtSwapchainCreateInfoKHR& create_info, 
            const void* pointer_to_next, VkSwapchainCreateFlagsKHR flags, VkSurfaceKHR surface, uint32_t min_image_count,
            VkFormat image_format, VkColorSpaceKHR image_color_space, VkExtent2D image_extent, uint32_t image_array_layers,
            VkImageUsageFlags image_usage, VkSharingMode image_sharing_mode, uint32_t queue_family_index_count, const uint32_t* pointer_to_queue_family_indicies,
            VkSurfaceTransformFlagBitsKHR pre_transform, VkCompositeAlphaFlagBitsKHR composite_alpha, VkPresentModeKHR present_mode,
            VkBool32 clipped, VkSwapchainKHR old_swapchain);
        void TransferSwapchainCreateInfo(CtSwapchainCreateInfoKHR& ct_create_info, VkSwapchainCreateInfoKHR& vk_create_info);

        void PopulateImageViewCreateInfo(CtImageViewCreateInfo& create_info,
            const void* pointer_to_next, VkImageViewCreateFlags flags, VkImage image, VkImageViewType view_type,
            VkFormat format, VkComponentMapping components, VkImageSubresourceRange subresource_range);
        void TransferImageViewCreateInfo(CtImageViewCreateInfo& ct_create_info, VkImageViewCreateInfo& vk_create_info);

        void InitializeSwapchain(CtSwapchainSupportDetails support_details);
        void InitializeSwapchainImageViews();

        VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> available_formats);
        VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& present_modes);
        VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

    friend class Engine;
    friend class CtDevice;
    friend class CtGraphicsPipeline;
};