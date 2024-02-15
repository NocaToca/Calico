#include <vulkan/vulkan.h>
#include "Engine.h"
#include "CtWindow.h"
#include <string>
#include <vector>
#include "CtInstance.h"
#include "CtDevice.h"
#include "CtSwapchain.h"
#include "CtGraphicsPipeline.h"
#include "CtRenderer.h"

#define CT_DEBUG


void Engine::StartEngine(EngineSettings settings){

    CreateObjects(settings);
    EngineLoop();
    Cleanup();
}

void Engine::CreateObjects(EngineSettings settings){
    CreateWindow(settings);
    CreateInstance(settings);
    CreateSurface(settings);
    CreateDevices(settings);
    CreateSwapchain(settings);
    CreateGraphicsPipeline(settings);
    CreateRenderer(settings);
}

void Engine::CreateGraphicsPipeline(EngineSettings settings){
    graphics_pipeline = CtGraphicsPipeline::CreateGraphicsPipeline(settings, devices, swapchain);
}

void Engine::CreateSwapchain(EngineSettings settings){
    swapchain = CtSwapchain::CreateSwapchain(this);
}

void Engine::CreateSurface(EngineSettings settings){
    window->CreateSurface(instance);
}

void Engine::CreateDevices(EngineSettings settings){
    devices = CtDevice::CreateDevice(this, settings);
}

void Engine::CreateWindow(EngineSettings settings){
    const std::string engine_name ("Calico");
    window = CtWindow::CreateWindow(settings.window_width, settings.window_height, engine_name);
}

void Engine::EngineLoop(){

    while(!window->ShouldWindowClose()){
        window->PollEvents();
        renderer->DrawFrame();
    }

}

void Engine::Cleanup(){
    window->Cleanup();
    free(window);
}

//This creates our Calico instance (Which is, again, an interface for our Vulkan interaction)
void Engine::CreateInstance(EngineSettings settings){
    instance = new CtInstance();

    //Right now we will just hard-code most values.
    CtInstanceApplicationInfo ct_applicaiton_info {};
    instance->CreateApplicationInfo(ct_applicaiton_info, nullptr, 
        "Calico", 
        VK_MAKE_VERSION(1, 0, 0), 
        "Calico Engine",
        VK_MAKE_VERSION(1, 0, 0),
        VK_API_VERSION_1_0);


    CtInstanceCreateInfo ct_create_info {}; 
    instance->CreateInstanceInfo(ct_create_info, nullptr, 0, 0, nullptr, 0, nullptr);

    #ifdef CT_DEBUG
    std::vector<const char*> validation_layers = {
        "VK_LAYER_KHRONOS_validation"
    };

    instance->EnableDebugInstance(ct_create_info, validation_layers);
    #endif

    instance->InitializeInstance(ct_applicaiton_info, ct_create_info);

    #ifdef CT_DEBUG
    instance->SetupDebugInstance();
    #endif
}

void Engine::CreateRenderer(EngineSettings settings){
    renderer = CtRenderer::CreateRenderer(settings, devices, swapchain, graphics_pipeline);
}