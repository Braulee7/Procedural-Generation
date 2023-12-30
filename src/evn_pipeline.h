#pragma once

#include "evn_device.h"

namespace evn {
    // struct to better organise the create info structs
    // for the pipeline
    struct PipelineConfigInfo {
        PipelineConfigInfo() = default;
        PipelineConfigInfo(const PipelineConfigInfo&) = delete;
        PipelineConfigInfo& operator=(const PipelineConfigInfo&) = delete;

        std::vector<VkVertexInputBindingDescription> binding_descriptions{};
        std::vector<VkVertexInputAttributeDescription> attribute_descriptions{};
        VkPipelineViewportStateCreateInfo viewport_info;
        VkPipelineInputAssemblyStateCreateInfo input_assembly_info;
        VkPipelineRasterizationStateCreateInfo raster_info;
        VkPipelineMultisampleStateCreateInfo multisample_info;
        VkPipelineColorBlendAttachmentState color_blend_attachment;
        VkPipelineColorBlendStateCreateInfo color_blending;
        VkPipelineDepthStencilStateCreateInfo depth_stencil_info;
        std::vector<VkDynamicState> dynamic_states;
        VkPipelineDynamicStateCreateInfo dynamic_state_info;
        VkPipelineLayout pipeline_layout = nullptr;
        VkRenderPass render_pass = nullptr;
        uint32_t subpass = 0;
    };

    class Pipeline {
    public:
        Pipeline(const std::string& vert_file_path,
                const std::string& frag_file_path,
                const PipelineConfigInfo& config);
        ~Pipeline();
        // delete copy constructor and assignment operators
        Pipeline(const Pipeline&) = delete;
        Pipeline& operator=(const Pipeline&) = delete;
        // helper methods
        static void defaultPipelineConfigInfo(PipelineConfigInfo& config);
    private: // methods
        static std::vector<char> readFile(const std::string& file_path);
        void createGraphicsPipeline(const std::string& vert_file_path,
                                    const std::string& frag_file_path,
                                    const PipelineConfigInfo& config);
        void createShaderModule(const std::vector<char>& code, VkShaderModule* shader_module);
    private:
        Device& r_device;
        VkPipeline m_graphics_pipeline;
        VkShaderModule m_vert_shader_module;
        VkShaderModule m_frag_shader_module;
    };
}