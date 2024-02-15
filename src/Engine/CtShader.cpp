#include "CtShader.h"
#include <iostream>
#include "CtDevice.h"
#include <fstream>

CtShader* CtShader::CreateShader(CtDevice* device, const std::string& shader_file_name, CtShaderPipelineStage pipeline_stage){
    CtShader* shader = new CtShader();

    shader->CreateShaderModule(device->GetInterfaceDevice(), shader_file_name);
    shader->pipeline_stage = pipeline_stage;

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

    // printf("Created shader module for %s.\n", shader_file_name);
}

void CtShader::CreateShaderPipelineInfo(VkPipelineShaderStageCreateInfo& vk_create_info){
    CtPipelineShaderStageCreateInfo ct_create_info {};
    PopulateShaderPipelineStageInfo(ct_create_info, nullptr, 0, shader_module, "main", nullptr);

    TransferShaderPipelineStageInfo(ct_create_info, vk_create_info);
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

void CtShader::PopulateShaderPipelineStageInfo(CtPipelineShaderStageCreateInfo& shader_pipeline_create_info,
            const void* pointer_to_next, VkPipelineShaderStageCreateFlags flags,  
            VkShaderModule module_, const char* pointer_to_name, const VkSpecializationInfo* pointer_to_specialization_info){

    shader_pipeline_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shader_pipeline_create_info.pNext = pointer_to_next;
    shader_pipeline_create_info.flags = flags;
    switch(pipeline_stage){
        case CT_SHADER_PIPELINE_STAGE_FRAGMENT:
            shader_pipeline_create_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            break;
        case CT_SHADER_PIPELINE_STAGE_VERTEX:
            shader_pipeline_create_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
            break;
        default:
            throw std::runtime_error("Shader stage not implemented.");
    }
    shader_pipeline_create_info.module_ = module_;
    shader_pipeline_create_info.pName = pointer_to_name;
    shader_pipeline_create_info.pSpecializationInfo = pointer_to_specialization_info;
}

void CtShader::TransferShaderPipelineStageInfo(CtPipelineShaderStageCreateInfo& ct_pipeline_shader_stage_create_info, VkPipelineShaderStageCreateInfo& vk_pipeline_shader_stage_create_info){
    vk_pipeline_shader_stage_create_info.sType = ct_pipeline_shader_stage_create_info.sType;
    vk_pipeline_shader_stage_create_info.pNext = ct_pipeline_shader_stage_create_info.pNext;
    vk_pipeline_shader_stage_create_info.flags = ct_pipeline_shader_stage_create_info.flags;
    vk_pipeline_shader_stage_create_info.stage = ct_pipeline_shader_stage_create_info.stage;
    vk_pipeline_shader_stage_create_info.module = ct_pipeline_shader_stage_create_info.module_;
    vk_pipeline_shader_stage_create_info.pName = ct_pipeline_shader_stage_create_info.pName;
    vk_pipeline_shader_stage_create_info.pSpecializationInfo = ct_pipeline_shader_stage_create_info.pSpecializationInfo;
}