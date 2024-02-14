#include <string>
#include <vector>
#include <vulkan/vulkan.h>

class CtDevice;
class CtGraphicsPipeline;

struct CtShaderModuleCreateInfo{

    //The structure type
    VkStructureType              sType;

    //The pointer to the structure extension
    const void*                  pNext;

    //Reserved for future use
    VkShaderModuleCreateFlags    flags;

    //The size of the following code array
    size_t                       codeSize;

    //The point to the compiled byte-code of the shader
    const uint32_t*              pCode;
};

struct CtPipelineShaderStageCreateInfo{
    //The structure type
    VkStructureType                     sType;
    //The structure extension pointer
    const void*                         pNext;
    //The flags to differentiate shader creation information
    VkPipelineShaderStageCreateFlags    flags;
    /*
        The flags can include:
        VK_PIPELINE_SHADER_STAGE_CREATE_ALLOW_VARYING_SUBGROUP_SIZE_BIT //Specifies that the subgroup size is allowed to vary in the shader stage
        VK_PIPELINE_SHADER_STAGE_CREATE_REQUIRE_FULL_SUBGROUPS_BIT      //Specifies that the subgroup sizes must be launched with all invocations active in the current stage
    */

    //This lets us know what stage we will be at
    VkShaderStageFlagBits               stage;
    /*
        These should be known, but for review the stages that we can create shaders for are:
        VK_SHADER_STAGE_VERTEX_BIT                  //The vertex stage
        VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT    //The tessellation control stage
        VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT //The evaluation stage for tessellation
        VK_SHADER_STAGE_GEOMETRY_BIT                //The geometry stage
        VK_SHADER_STAGE_FRAGMENT_BIT                //The fragment stage
        VK_SHADER_STAGE_COMPUTE_BIT                 //The compute stage (Compute shaders!)
        VK_SHADER_STAGE_ALL_GRAPHICS                //Uses all the graphics stages

        I wont go over the other stage extensions since I haven't looked into them much
    */

    //The shader module
    VkShaderModule                      module_;

    //The name of the entrance function
    const char*                         pName;

    //The specialization information defined by the structure
    const VkSpecializationInfo*         pSpecializationInfo;
};

//Most simply used to differentiate what stage this shader is going to work with
enum CtShaderPipelineStage{
    CT_SHADER_PIPELINE_STAGE_FRAGMENT,
    CT_SHADER_PIPELINE_STAGE_VERTEX
};


class CtShader{

    

    public:
        static CtShader* CreateShader(CtDevice* device, const std::string& shader_file_name, CtShaderPipelineStage pipeline_stage);
        VkPipelineShaderStageCreateInfo CreateShaderPipelineInfo();

    private:

        VkShaderModule shader_module;
        CtShaderPipelineStage pipeline_stage;

        std::vector<char> ReadFile(const std::string& file_name);

        void PopulateShaderModuleCreateInfo(CtShaderModuleCreateInfo& shader_create_info,
            const void* pointer_to_next, VkShaderModuleCreateFlags flags,
            size_t code_size, const uint32_t* pointer_to_code);
        void TransferShaderModuleCreateInfo(CtShaderModuleCreateInfo& ct_shader_create_info, VkShaderModuleCreateInfo& vk_shader_create_info);

        void PopulateShaderPipelineStageInfo(CtPipelineShaderStageCreateInfo& shader_pipeline_create_info,
            const void* pointer_to_next, VkPipelineShaderStageCreateFlags flags, 
            VkShaderModule module_, const char* pointer_to_name, const VkSpecializationInfo* pointer_to_specialization_info);
        void TransferShaderPipelineStageInfo(CtPipelineShaderStageCreateInfo& ct_shader_pipeline_create_info, VkPipelineShaderStageCreateInfo& vk_shader_pipeline_create_info);

        void CreateShaderModule(VkDevice* interface_device, const std::string& file_name);

        void DestroyShaderModule(VkDevice* interface_device){
            vkDestroyShaderModule(*interface_device, shader_module, nullptr);
        }

    friend class CtGraphicsPipeline;

};