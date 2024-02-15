#include <vulkan/vulkan.h>
#include <vector>

class CtDevice;
struct EngineSettings;
class CtSwapchain;
class CtGraphicsPipeline;

//This class is responsible for drawing as well as doing the synch variables in check
class CtRenderer{

    public:
        static CtRenderer* CreateRenderer(EngineSettings settings, CtDevice* device, CtSwapchain* swapchain, CtGraphicsPipeline* graphics_pipeline);

        void DrawFrame();

    private:

        bool framebuffer_resized = false;

        //We're actually going to hold our device here, so we don't need to pass it every frame
        CtDevice* device;
        CtSwapchain* swapchain;
        CtGraphicsPipeline* graphics_pipeline;

        VkCommandPool command_pool;

        std::vector<VkCommandBuffer> command_buffers;
        std::vector<VkSemaphore> image_available_semaphores;
        std::vector<VkSemaphore> render_finished_semaphores;
        std::vector<VkFence> in_flight_fences;

        //Buffers
        VkBuffer vertex_buffer;
        VkDeviceMemory vertex_buffer_memory;

        VkBuffer index_buffer;
        VkDeviceMemory index_buffer_memory;

        uint32_t max_frames_in_flight; //Just a quick reference

        uint32_t current_frame;

        void CreateSyncObjects();
        void CreateCommandBuffers();
        void CreateCommandPool();
        void CreateVertexBuffer();
        void CreateIndexBuffer();

        void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer, VkDeviceMemory &buffer_memory);
        void CopyBuffer(VkBuffer source_buffer, VkBuffer destination_buffer, VkDeviceSize size);

        VkCommandBuffer BeginSingleTimeCommands();
        void EndSingleTimeCommands(VkCommandBuffer command_buffer);

        void RecordCommandBuffer(VkCommandBuffer command_buffer, uint32_t image_index);

    friend class CtSwapchain;
    friend class CtDevice;
    friend class CtQueueFamily;
    friend class CtGraphicsPipeline;
    friend class CtSwapchain;

};