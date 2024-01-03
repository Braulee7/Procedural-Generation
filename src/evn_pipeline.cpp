#include "evn_pipeline.h"

namespace evn {
        Pipeline::Pipeline(Device& device, 
                           const std::string& vert_file_path,
                           const std::string& frag_file_path,
                           const PipelineConfigInfo& config)
            : r_device(device)
        {
            createGraphicsPipeline(vert_file_path, frag_file_path, config);
        }

        Pipeline::~Pipeline()
        {
            vkDestroyPipeline(r_device.device(), m_graphics_pipeline, nullptr);
        }

        void Pipeline::createGraphicsPipeline(const std::string& vert_file_path,
                           const std::string& frag_file_path,
                           const PipelineConfigInfo& config)
        {
            auto frag_code {readFile(frag_file_path)};
            auto vert_code {readFile(vert_file_path)};
            VkShaderModule frag_module { createShaderModule(frag_code) };
            VkShaderModule vert_module { createShaderModule(vert_code) };
            VkPipelineShaderStageCreateInfo vert_create_info{};
            VkPipelineShaderStageCreateInfo frag_create_info{};

            // create the pipeline shaders
            vert_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            vert_create_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
            vert_create_info.module = vert_module;
            vert_create_info.pName = "main";
        
            frag_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            frag_create_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            frag_create_info.module = frag_module;
            frag_create_info.pName = "main";

            VkPipelineShaderStageCreateInfo shader_stages[] = {vert_create_info, frag_create_info};
            // fixed functions

            auto& binding_desc {config.binding_descriptions};
            auto& attrib_desc {config.attribute_descriptions};

            VkPipelineVertexInputStateCreateInfo vertex_input_info = {};
            vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            vertex_input_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(attrib_desc.size());
            vertex_input_info.pVertexAttributeDescriptions = attrib_desc.data();
            vertex_input_info.vertexBindingDescriptionCount = 1;
            vertex_input_info.pVertexBindingDescriptions = &binding_desc;

            VkGraphicsPipelineCreateInfo pipeline_info{};
            pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
            pipeline_info.stageCount = 2;
            pipeline_info.pStages = shader_stages;
            pipeline_info.pVertexInputState = &vertex_input_info;
            pipeline_info.pInputAssemblyState = &config.input_assembly_info;
            pipeline_info.pViewportState = &config.viewport_info;
            pipeline_info.pRasterizationState = &config.raster_info;
            pipeline_info.pMultisampleState = &config.multisample_info;
            pipeline_info.pColorBlendState = &config.color_blending;
            pipeline_info.pDepthStencilState = &config.depth_stencil_info;
            pipeline_info.pDynamicState = &config.dynamic_state_info;

            pipeline_info.layout = config.pipeline_layout;
            pipeline_info.renderPass = config.render_pass;
            pipeline_info.subpass = config.subpass;

            pipeline_info.basePipelineIndex = -1;
            pipeline_info.basePipelineHandle = VK_NULL_HANDLE;

            if (vkCreateGraphicsPipelines(r_device.device(), VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &m_graphics_pipeline)!=VK_SUCCESS)
                throw std::runtime_error("Failed to create grahpics pipeline");
            
            // clean up shader modules
            vkDestroyShaderModule(r_device.device(), frag_module, nullptr);
            vkDestroyShaderModule(r_device.device(), vert_module, nullptr);
        }

        VkShaderModule Pipeline::createShaderModule(const std::vector<char>& code)
        {
            VkShaderModuleCreateInfo create_info{};
            VkShaderModule module;
            // create the information
            create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            create_info.codeSize = code.size();
            create_info.pCode = reinterpret_cast<const uint32_t*>(code.data());
            
            // create the shader
            if (vkCreateShaderModule(r_device.device(), &create_info, nullptr, &module) != VK_SUCCESS)
                throw std::runtime_error("failed to create the shader module");
            return module;
        }

        std::vector<char> Pipeline::readFile(const std::string& file_name)
        {
            // open the file having the pointer be at the end to read in the 
            // size and be in binary to not have to deal with text transformations

            std::ifstream file(file_name, std::ios::ate | std::ios::binary);
            if (!file.is_open())
                throw std::runtime_error("Failed to load in file: " + file_name);

            size_t file_size{ (size_t)file.tellg() };
            std::cout << file_size << std::endl;
            std::vector<char> buffer(file_size);

            // read the file
            file.seekg(0);
            file.read(buffer.data(), file_size);
            file.close();
            return buffer;
        }

        void Pipeline::defaultPipelineConfigInfo(PipelineConfigInfo& config)
        {
            // vertex attributes
            config.binding_descriptions = Vertex::getBindingDesc();
            config.attribute_descriptions = Vertex::getAttributes();


            // putting these values in the dynamic state will ignore them during 
            // pipeline creation and force them to be required during draw time
            config.dynamic_states = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
            config.dynamic_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
            config.dynamic_state_info.dynamicStateCount = static_cast<uint32_t>(config.dynamic_states.size());
            config.dynamic_state_info.pDynamicStates = config.dynamic_states.data();

            // input assembly
            config.input_assembly_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
            config.input_assembly_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            config.input_assembly_info.primitiveRestartEnable = VK_FALSE;

            config.viewport_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
            config.viewport_info.scissorCount = 1;
            config.viewport_info.viewportCount = 1;
          
            // rasteriser
            config.raster_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
            config.raster_info.depthClampEnable = VK_FALSE;
            config.raster_info.rasterizerDiscardEnable = VK_FALSE;
            config.raster_info.polygonMode = VK_POLYGON_MODE_FILL;
            config.raster_info.lineWidth = 1.0f;
            config.raster_info.cullMode = VK_CULL_MODE_BACK_BIT;
            config.raster_info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
            config.raster_info.depthBiasEnable = VK_FALSE;
            config.raster_info.depthBiasClamp = 0.0f;
            config.raster_info.depthBiasConstantFactor = 0.0f;
            config.raster_info.depthBiasSlopeFactor = 0.0f;

            // multisampling
            // TODO disbled for now, feature for antialiasing
            config.multisample_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
            config.multisample_info.sampleShadingEnable = VK_FALSE;
            config.multisample_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
            config.multisample_info.minSampleShading = 1.0f; // Optional
            config.multisample_info.pSampleMask = nullptr; // Optional
            config.multisample_info.alphaToCoverageEnable = VK_FALSE; // Optional
            config.multisample_info.alphaToOneEnable = VK_FALSE; // Optional
            
            // color blend: requires two structs
            // determines how the colors of the new frame buffer
            // will blend with the previous frame buffers. Can be 
            // disabled. This configuration will implement alpha blending
            // which blends the colors based on opacity
            config.color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
            config.color_blend_attachment.blendEnable = VK_TRUE;
            config.color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA; // Optional
            config.color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA; // Optional
            config.color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
            config.color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
            config.color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
            config.color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

            config.color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
            config.color_blending.logicOpEnable = VK_FALSE;
            config.color_blending.logicOp = VK_LOGIC_OP_COPY;
            config.color_blending.attachmentCount = 1;
            config.color_blending.pAttachments = &config.color_blend_attachment;
            for (int i{ 0 }; i < 4; i++) config.color_blending.blendConstants[i] = 0.0f;
        }
}