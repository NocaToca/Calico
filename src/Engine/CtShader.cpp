#include "CtShader.h"
#include <iostream>
#include "CtDevice.h"
#include <fstream>

CtShader* CtShader::CreateShader(CtDevice* device, const std::string& shader_file_name){
    CtShader* shader = new CtShader();

    shader->CreateShaderModule(device->GetInterfaceDevice(), shader_file_name);

    return shader;
}

std::vector<char> CtShader::ReadFile(const std::string& file_name){
    std::ifstream file(file_name, std::ios::ate | std::ios::binary);

    if(!file.is_open()){
        throw std::runtime_error("Failed to open file");
    }

    size_t file_size = (size_t) file.tellg();
    std::vector<char> buffer(file_size);

    file.seekg(0);
    file.read(buffer.data(), file_size);

    file.close();
    return buffer;
}

void CtShader::CreateShaderModule(VkDevice* interface_device, const std::string& shader_file_name){
    std::vector<char> byte_code = ReadFile(shader_file_name);

    CtShaderModuleCreateInfo ct_create_info {};
    PopulateShaderModuleCreateInfo(ct_create_info, nullptr, 0, byte_code.size(), reinterpret_cast<const uint32_t*>(byte_code.data()));
    
    VkShaderModuleCreateInfo vk_create_info {};
    TransferShaderModuleCreateInfo(ct_create_info, vk_create_info);

    if(vkCreateShaderModule(*interface_device, &vk_create_info, nullptr, &shader_module) != VK_SUCCESS){
        throw std::runtime_error("Failed to create shader module.");
    }
}

void CtShader::PopulateShaderModuleCreateInfo(CtShaderModuleCreateInfo& shader_create_info,
            const void* pointer_to_next, VkShaderModuleCreateFlags flags,
            size_t code_size, const uint32_t* pointer_to_code){
    
    shader_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shader_create_info.pNext = pointer_to_next;
    shader_create_info.flags = flags;
    shader_create_info.codeSize = code_size;
    shader_create_info.pCode = pointer_to_code;

}

void CtShader::TransferShaderModuleCreateInfo(CtShaderModuleCreateInfo& ct_shader_module_create_info, VkShaderModuleCreateInfo& vk_shader_module_create_info){
    vk_shader_module_create_info.sType = ct_shader_module_create_info.sType;
    vk_shader_module_create_info.pNext = ct_shader_module_create_info.pNext;
    vk_shader_module_create_info.flags = ct_shader_module_create_info.flags;
    vk_shader_module_create_info.codeSize = ct_shader_module_create_info.codeSize;
    vk_shader_module_create_info.pCode = ct_shader_module_create_info.pCode;
}