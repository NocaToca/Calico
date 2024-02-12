
#include "src/Engine/Engine.h"
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
    settings.window_width = WIDTH;
    settings.window_height = HEIGHT;

    try{
        engine.StartEngine(settings);
    } catch(const std::exception& exception){
        std::cerr << exception.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;

}