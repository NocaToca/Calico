#include <cstdint>

class CtWindow;
class CtInstance;
class CtDevice;
class CtSwapchain;

struct EngineSettings{

    uint32_t window_width;
    uint32_t window_height;

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

    friend class CtDevice;
    friend class CtSwapchain;
};