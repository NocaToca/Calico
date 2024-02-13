#include <string>
#include <cstdint>
#include <vulkan/vulkan.h>

class GLFWwindow;
class CtInstance;

class CtWindow{

    public:

        static CtWindow* CreateWindow(uint32_t width, uint32_t height, std::string name);
        bool ShouldWindowClose();
        void PollEvents();
        void Cleanup();
        VkSurfaceKHR* GetSurface();
        void CreateSurface(CtInstance* instance);
        GLFWwindow* GetWindow();

    private:

        uint32_t initial_height;
        uint32_t initial_width;

        GLFWwindow* window;

        VkSurfaceKHR surface;

        CtWindow();

};