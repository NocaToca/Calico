#include <string>
#include <vector>
#include <vulkan/vulkan.h>

class CtDevice;
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


class CtShader{

    VkShaderModule shader_module;

    public:
        static CtShader* CreateShader(CtDevice* device, const std::string& shader_file_name);

    private:
        std::vector<char> ReadFile(const std::string& file_name);

        void PopulateShaderModuleCreateInfo(CtShaderModuleCreateInfo& shader_create_info,
            const void* pointer_to_next, VkShaderModuleCreateFlags flags,
            size_t code_size, const uint32_t* pointer_to_code);
        void TransferShaderModuleCreateInfo(CtShaderModuleCreateInfo& ct_shader_create_info, VkShaderModuleCreateInfo& vk_shader_create_info);

        void CreateShaderModule(VkDevice* interface_device, const std::string& file_name);

};