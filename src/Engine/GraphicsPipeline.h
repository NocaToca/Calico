

class GraphicsPipeline {

    public:


    private:

        std::vector<VkPipelineShaderCreateInfo> createShaderInformation(std::vector<std::string>> file_paths);

        VkPipelineVertexInputStateCreateInfo createVertexInformation();

        VkPipelineInputAssemblyStateCreateInfo createInputAssembly();

}