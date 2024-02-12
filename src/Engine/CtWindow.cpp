#include "CtWindow.h"
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include "CtInstance.h"

CtWindow::CtWindow(){

}

//Creates our initial window
CtWindow* CtWindow::CreateWindow(uint32_t width, uint32_t height, const std::string name){

    CtWindow* ct_window = new CtWindow();

    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    GLFWwindow* window = glfwCreateWindow(width, height, name.c_str(), nullptr, nullptr);

    //To allow for debug operations
    glfwSetWindowUserPointer(window, ct_window);

    //For frame buffer callbacks
    // glfwSetFramebufferSizeCallback(window, )

    ct_window->initial_height = height;
    ct_window->initial_width = width;
    ct_window->window = window;

    return ct_window;
}

bool CtWindow::ShouldWindowClose(){
    return glfwWindowShouldClose(window);
}

//this doesn't need to be here, but I want direct interaction between my classes, so this serves as seperation
void CtWindow::PollEvents(){
    glfwPollEvents();
}

void CtWindow::Cleanup(){
    glfwDestroyWindow(window);
    glfwTerminate();
}

void CtWindow::CreateSurface(CtInstance* instance){
    if(glfwCreateWindowSurface(*(instance->GetInstance()), window, nullptr, &surface) != VK_SUCCESS){
        throw std::runtime_error("Window surface creation failed.");
    }
}

VkSurfaceKHR* CtWindow::GetSurface(){
    return &surface;
}