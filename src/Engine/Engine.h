#include <cstdint>
#include <string>
#include <vector>

class CtWindow;
class CtInstance;
class CtDevice;
class CtSwapchain;
class CtGraphicsPipeline;
class CtRenderer;

struct WindowSettings{
    uint32_t window_width;
    uint32_t window_height;
};

struct GraphicsSettings{
    std::vector<std::string> shader_files; 
    std::vector<uint32_t> shader_stages;
    uint32_t max_frames_in_flight;
};

struct EngineSettings{

    WindowSettings windows_settings;
    GraphicsSettings graphics_settings;

};


class Engine{

    public:
        void StartEngine(EngineSettings settings);

    private:
        //Members
        CtWindow* window;

        //Instance
        CtInstance* instance;

        //Devices
        CtDevice* devices;

        //Swapchain
        CtSwapchain* swapchain;

        //Pipeline
        CtGraphicsPipeline* graphics_pipeline;

        //The actual Renderer
        CtRenderer* renderer;

        //Functions
        void EngineLoop();
        void Cleanup();
        void CreateObjects(EngineSettings settings);
        void CreateWindow(EngineSettings settings);
        void CreateRenderer(EngineSettings settings);
        void CreateInstance(EngineSettings settings);
        void CreateDevices(EngineSettings settings);
        void CreateSurface(EngineSettings settings);
        void CreateSwapchain(EngineSettings settings);
        void CreateGraphicsPipeline(EngineSettings settings);

    friend class CtDevice;
    friend class CtSwapchain;
};