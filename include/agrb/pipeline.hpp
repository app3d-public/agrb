#pragma once

#include <acul/functional/unique_function.hpp>
#include <acul/io/path.hpp>
#include <acul/list.hpp>
#include <acul/memory/destructible.hpp>
#include <acul/op_result.hpp>
#include <umbf/umbf.hpp>
#include "device.hpp"

#define AGRB_TYPE_ID_SHADER 0x7559
#define AGRB_SIGN_ID_SHADER 0x78C7C6EC

namespace agrb
{
    // Configuration settings for a Vulkan graphics pipeline.
    struct pipeline_config_base
    {
        acul::vector<vk::PipelineShaderStageCreateInfo> shader_stages;
        vk::PipelineLayout pipeline_layout = nullptr;
    };

    template <typename T>
    struct pipeline_config;

    using graphics_config = pipeline_config<vk::GraphicsPipelineCreateInfo>;
    using compute_config = pipeline_config<vk::ComputePipelineCreateInfo>;

    template <>
    struct APPLIB_API pipeline_config<vk::GraphicsPipelineCreateInfo> final : pipeline_config_base
    {
        vk::PipelineViewportStateCreateInfo viewport_info;
        vk::PipelineInputAssemblyStateCreateInfo input_assembly_info;
        vk::PipelineRasterizationStateCreateInfo rasterization_info;
        vk::PipelineMultisampleStateCreateInfo multisample_info;
        vk::PipelineColorBlendAttachmentState color_blend_attachment;
        vk::PipelineColorBlendStateCreateInfo color_blend_info;
        vk::PipelineDepthStencilStateCreateInfo depth_stencil_info;
        vk::PipelineRasterizationConservativeStateCreateInfoEXT conservative_state_info;
        acul::vector<vk::DynamicState> dynamic_state_enables;
        vk::PipelineDynamicStateCreateInfo dynamic_state_info;
        acul::vector<vk::VertexInputBindingDescription> binding_descriptions;
        acul::vector<vk::VertexInputAttributeDescription> attribute_descriptions;
        vk::PipelineVertexInputStateCreateInfo vertex_input_info;
        vk::SpecializationInfo specialization_info;
        acul::vector<vk::SpecializationMapEntry> specialization_map;
        vk::RenderPass render_pass = nullptr;
        u32 subpass = 0;

        /**
         * @brief Load default pipeline configuration settings.
         *
         * This method initializes the pipeline configuration with default values
         * suitable for most graphics pipelines.
         *
         * @return Reference to the updated pipeline_config object.
         */
        pipeline_config &load_defaults();

        /**
         * @brief Enable alpha blending in the pipeline configuration.
         *
         * This method configures the pipeline to enable alpha blending, which is
         * typically used for transparency effects.
         *
         * @return Reference to the updated pipeline_config object.
         */
        pipeline_config &enable_alpha_blending();

        /**
         * @brief Enable multi-sample anti-aliasing (MSAA) in the pipeline configuration.
         * @param msaa MSAA Levelt
         * @param sample_shading Sample Shading. If set to 0.0 (default) then no apply this option
         * @return Reference to the updated pipeline_config object.
         */
        pipeline_config &enable_msaa(vk::SampleCountFlagBits msaa, f32 sample_shading = 0.0f)
        {
            multisample_info.setRasterizationSamples(msaa);
            if (msaa > vk::SampleCountFlagBits::e1 && sample_shading != 0.0f)
                multisample_info.setSampleShadingEnable(true).setMinSampleShading(sample_shading);
            return *this;
        }
    };

    template <>
    struct pipeline_config<vk::ComputePipelineCreateInfo> final : public pipeline_config_base
    {
    };

    struct shader_block final : umbf::Block
    {
        u64 id;
        acul::vector<char> code;

        virtual u32 signature() const override { return AGRB_SIGN_ID_SHADER; }
    };

    /**
     * @brief Struct to represent a Vulkan shader module.
     *
     * This struct contains information about a shader module, including its file path,
     * the Vulkan shader module object, and the shader code.
     */
    struct APPLIB_API shader_module
    {
        acul::shared_ptr<shader_block> data; ///< Shader data block.
        vk::ShaderModule module;             ///< Vulkan shader module object.

        /**
         * @brief Load the shader module from the specified device.
         *
         * This method loads the shader module into the Vulkan device, compiling the shader code
         * and creating a Vulkan shader module object.
         *
         * @param device The Vulkan device to load the shader module into.
         */
        bool load(device &device);

        /**
         * @brief Destroy the shader module.
         *
         * This method destroys the Vulkan shader module object associated with this ShaderModule.
         *
         * @param device The Vulkan device that contains the shader module.
         */
        void destroy(device &device)
        {
            device.vk_device.destroyShaderModule(module, nullptr, device.loader);
            module = nullptr;
        }
    };

    using shader_cache = acul::hashmap<u64, shader_module>;

    /**
     * @brief Loads a shader library into the given shader cache.
     *
     * This function loads a shader library from the given file path into the given shader cache.
     * If the file cannot be read or if the file is not a valid shader library, then an error is returned.
     * If a shader with the same ID already exists in the cache, then the shader is replaced with the new one.
     *
     * @param library_path The file path to the shader library to load.
     * @param cache The shader cache to load the shader library into.
     *
     * @return An acul::op_result indicating the success or failure of the operation.
     * If the operation failed, the result contains an acul::op_error with the appropriate error code and domain.
     */
    APPLIB_API acul::op_result load_shader_library(const acul::path &library_path, shader_cache &cache);

    /**
     * @brief Get a shader module from the cache, loading it from the specified library if it isn't already cached.
     * @param id The ID of the shader module to retrieve.
     * @param out[out] The shader module to return.
     * @param cache The cache to use.
     * @param device The device to load the shader module with.
     * @param library_path The path to the library to load from if the shader module is not already cached.
     * @return An op_result indicating success or failure with a code from @ref agrb_op_error_codes.
     */
    APPLIB_API acul::op_result get_shader(u64 id, vk::ShaderModule &out, shader_cache &cache, device &device,
                                          const acul::path &library_path);

    /**
     * @brief Clear the shader cache by destroying all shader modules and clearing the cache.
     *
     * This method destroys all shader modules associated with the given shader cache and clears the cache.
     * It should be called when the shader cache is no longer needed.
     *
     * @param device The device to destroy the shader modules with.
     * @param cache The shader cache to clear.
     */
    inline void clear_shader_cache(agrb::device &device, shader_cache &cache)
    {
        for (auto &item : cache)
            if (item.second.module) item.second.destroy(device);
        cache.clear();
    }

    template <typename T>
    struct pipeline_batch;

    namespace details
    {
        template <typename T>
        struct artifact
        {
            template <typename V>
            using custom_data_t = acul::destructible_value<V, artifact &>;

            pipeline_config<T> config;
            T create_info;
            acul::destructible_data<artifact &> *tmp = nullptr;
            acul::unique_function<void(vk::Pipeline)> commit = nullptr;
        };

        template <typename T>
        struct pipeline_artifact_configure
        {
            static_assert(sizeof(T) == 0, "pipeline_artifact_configure<>: unsupported pipeline type");
        };

        template <>
        struct pipeline_artifact_configure<vk::GraphicsPipelineCreateInfo>
        {
            using callback_type = bool (*)(artifact<vk::GraphicsPipelineCreateInfo> &, vk::RenderPass,
                                           vk::PipelineLayout &, device &);
        };

        template <>
        struct pipeline_artifact_configure<vk::ComputePipelineCreateInfo>
        {
            using callback_type = bool (*)(artifact<vk::ComputePipelineCreateInfo> &, vk::PipelineLayout &, device &);
        };
    } // namespace details

    /**
     * @brief Template struct for batching Vulkan pipeline creation.
     *
     * This struct facilitates the batch creation of Vulkan pipelines. The template parameter
     * specifies the type of pipeline being created
     *
     * @tparam T The type of pipeline create info.
     */
    template <typename T>
    struct pipeline_batch
    {
        using create_info = T;
        using artifact = details::artifact<T>;
        using PFN_configure_artifact = typename details::pipeline_artifact_configure<create_info>::callback_type;
        acul::list<artifact> artifacts; ///< Stores configurations for each pipeline.
        vk::PipelineCache cache;        ///< Pipeline cache used for pipeline creation.

        /**
         * @brief Creates Vulkan pipelines in a batch operation.
         *
         * This method uses the stored create info structures to create multiple Vulkan pipelines at once.
         * After creation, it updates the pointers to the newly created pipeline objects and cleans up any
         * temporary shader modules.
         *
         * @param device The Vulkan device to use for pipeline creation.
         * @return True if all pipelines were successfully created, otherwise false.
         */
        bool allocate_pipelines(device &device, size_t size)
        {
            vk::Pipeline pipelines[size];
            create_info create_info[size];
            auto it = artifacts.begin();
            for (size_t i = 0; i < size; i++, ++it) create_info[i] = it->create_info;

            vk::Result res;
            if constexpr (std::is_same<T, vk::GraphicsPipelineCreateInfo>::value)
                res = device.vk_device.createGraphicsPipelines(cache, size, create_info, nullptr, pipelines,
                                                               device.loader);
            else
                res = device.vk_device.createComputePipelines(cache, size, create_info, nullptr, pipelines,
                                                              device.loader);
            if (res != vk::Result::eSuccess) return false;

            it = artifacts.begin();
            for (size_t i = 0; i < size; i++, ++it)
            {
                if (it->commit)
                    it->commit(pipelines[i]);
                else
                    device.vk_device.destroyPipeline(pipelines[i], nullptr, device.loader);
            }
            return true;
        }

        ~pipeline_batch()
        {
            for (auto &a : artifacts) acul::release(a.tmp);
        }
    };

    using graphics_pipeline_batch = pipeline_batch<vk::GraphicsPipelineCreateInfo>;
    using compute_pipeline_batch = pipeline_batch<vk::ComputePipelineCreateInfo>;

    /**
     * @brief Prepares a basic graphics pipeline.
     *
     * This function sets up the basic configuration for a graphics pipeline, suitable for simple
     * operations such as rendering a full-screen quad. It configures the pipeline with the provided
     * vertex and fragment shaders, and sets default states for various pipeline stages.
     *
     * @param config The pipeline configuration to be filled in.
     * @param create_info The create info structure for the graphics pipeline.
     * @param shaders An array of shader modules to be used in the pipeline as [vertex, fragment].
     * @param device The Vulkan device to be used for pipeline creation.
     */
    APPLIB_API void prepare_base_graphics_pipeline(pipeline_batch<vk::GraphicsPipelineCreateInfo>::artifact &artifact,
                                                   vk::ShaderModule *shaders, device &device);

    APPLIB_API void
    configure_compute_pipeline_artifact(pipeline_batch<vk::ComputePipelineCreateInfo>::artifact &artifact,
                                        vk::PipelineLayout &layout, agrb::device &device,
                                        const vk::ShaderModule &shader);

    namespace streams
    {
        APPLIB_API void write_shader(acul::bin_stream &stream, umbf::Block *block);
        APPLIB_API umbf::Block *read_shader(acul::bin_stream &stream);
        inline umbf::streams::Stream shader = {read_shader, write_shader};
    } // namespace streams
} // namespace agrb
