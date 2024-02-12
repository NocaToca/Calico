#include <string>
#include <cstdint>

class GLFWwindow;

class CtWindow{

    public:

        static CtWindow* CreateWindow(uint32_t width, uint32_t height, std::string name);

        bool ShouldWindowClose();

        void PollEvents();

        void Cleanup();

    private:

        uint32_t initial_height;
        uint32_t initial_width;

        GLFWwindow* window;

        CtWindow();

};