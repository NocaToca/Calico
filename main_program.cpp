
#include "src/Engine/Engine.h"
#include "src/Engine/CtShader.h"
#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <cstdint>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

int main(){

    Engine engine;

    EngineSettings settings {};
    WindowSettings window_settings {};
    window_settings.window_width = WIDTH;
    window_settings.window_height = HEIGHT;

    std::string fragment("C:/Calico/Shaders/frag.spv");
    std::string vertex("C:/Calico/Shaders/vert.spv");

    GraphicsSettings graphic_settings {};
    graphic_settings.shader_files = {fragment, vertex};
    graphic_settings.shader_stages = {static_cast<uint32_t>(CT_SHADER_PIPELINE_STAGE_FRAGMENT), static_cast<uint32_t>(CT_SHADER_PIPELINE_STAGE_VERTEX)};
    graphic_settings.max_frames_in_flight = 2;

    settings.windows_settings = window_settings;
    settings.graphics_settings = graphic_settings;

    try{
        engine.StartEngine(settings);
    } catch(const std::exception& exception){
        std::cerr << exception.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;

}