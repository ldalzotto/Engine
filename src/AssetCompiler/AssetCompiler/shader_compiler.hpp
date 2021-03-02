
#include "glslang/Include/glslang_c_interface.h"

#ifdef GPU_DEBUG
#define sc_handle_error(glslang_program_t_ptr, glslang_shader_s_ptr, Code) ShaderCompiler_ErrorHandler((glslang_shader_s_ptr), (glslang_program_t_ptr), (Code))

#define sc_handle_error_silent(glslang_program_t_ptr, glslang_shader_s_ptr, Code)                                                                                                                      \
    {                                                                                                                                                                                                  \
        if (!ShaderCompiler_ErrorHandler_silent((glslang_shader_s_ptr), (glslang_program_t_ptr), (Code)))                                                                                              \
        {                                                                                                                                                                                              \
            return 0;                                                                                                                                                                                  \
        }                                                                                                                                                                                              \
    }

#else
#define sc_handle_error(ShaderCompiledPtr, ShaderPtr, Code) Code
#endif

#include "glslang/Include/ResourceLimits.h"

static TBuiltInResource InitResources()
{
    TBuiltInResource Resources;

    Resources.maxLights = 32;
    Resources.maxClipPlanes = 6;
    Resources.maxTextureUnits = 32;
    Resources.maxTextureCoords = 32;
    Resources.maxVertexAttribs = 64;
    Resources.maxVertexUniformComponents = 4096;
    Resources.maxVaryingFloats = 64;
    Resources.maxVertexTextureImageUnits = 32;
    Resources.maxCombinedTextureImageUnits = 80;
    Resources.maxTextureImageUnits = 32;
    Resources.maxFragmentUniformComponents = 4096;
    Resources.maxDrawBuffers = 32;
    Resources.maxVertexUniformVectors = 128;
    Resources.maxVaryingVectors = 8;
    Resources.maxFragmentUniformVectors = 16;
    Resources.maxVertexOutputVectors = 16;
    Resources.maxFragmentInputVectors = 15;
    Resources.minProgramTexelOffset = -8;
    Resources.maxProgramTexelOffset = 7;
    Resources.maxClipDistances = 8;
    Resources.maxComputeWorkGroupCountX = 65535;
    Resources.maxComputeWorkGroupCountY = 65535;
    Resources.maxComputeWorkGroupCountZ = 65535;
    Resources.maxComputeWorkGroupSizeX = 1024;
    Resources.maxComputeWorkGroupSizeY = 1024;
    Resources.maxComputeWorkGroupSizeZ = 64;
    Resources.maxComputeUniformComponents = 1024;
    Resources.maxComputeTextureImageUnits = 16;
    Resources.maxComputeImageUniforms = 8;
    Resources.maxComputeAtomicCounters = 8;
    Resources.maxComputeAtomicCounterBuffers = 1;
    Resources.maxVaryingComponents = 60;
    Resources.maxVertexOutputComponents = 64;
    Resources.maxGeometryInputComponents = 64;
    Resources.maxGeometryOutputComponents = 128;
    Resources.maxFragmentInputComponents = 128;
    Resources.maxImageUnits = 8;
    Resources.maxCombinedImageUnitsAndFragmentOutputs = 8;
    Resources.maxCombinedShaderOutputResources = 8;
    Resources.maxImageSamples = 0;
    Resources.maxVertexImageUniforms = 0;
    Resources.maxTessControlImageUniforms = 0;
    Resources.maxTessEvaluationImageUniforms = 0;
    Resources.maxGeometryImageUniforms = 0;
    Resources.maxFragmentImageUniforms = 8;
    Resources.maxCombinedImageUniforms = 8;
    Resources.maxGeometryTextureImageUnits = 16;
    Resources.maxGeometryOutputVertices = 256;
    Resources.maxGeometryTotalOutputComponents = 1024;
    Resources.maxGeometryUniformComponents = 1024;
    Resources.maxGeometryVaryingComponents = 64;
    Resources.maxTessControlInputComponents = 128;
    Resources.maxTessControlOutputComponents = 128;
    Resources.maxTessControlTextureImageUnits = 16;
    Resources.maxTessControlUniformComponents = 1024;
    Resources.maxTessControlTotalOutputComponents = 4096;
    Resources.maxTessEvaluationInputComponents = 128;
    Resources.maxTessEvaluationOutputComponents = 128;
    Resources.maxTessEvaluationTextureImageUnits = 16;
    Resources.maxTessEvaluationUniformComponents = 1024;
    Resources.maxTessPatchComponents = 120;
    Resources.maxPatchVertices = 32;
    Resources.maxTessGenLevel = 64;
    Resources.maxViewports = 16;
    Resources.maxVertexAtomicCounters = 0;
    Resources.maxTessControlAtomicCounters = 0;
    Resources.maxTessEvaluationAtomicCounters = 0;
    Resources.maxGeometryAtomicCounters = 0;
    Resources.maxFragmentAtomicCounters = 8;
    Resources.maxCombinedAtomicCounters = 8;
    Resources.maxAtomicCounterBindings = 1;
    Resources.maxVertexAtomicCounterBuffers = 0;
    Resources.maxTessControlAtomicCounterBuffers = 0;
    Resources.maxTessEvaluationAtomicCounterBuffers = 0;
    Resources.maxGeometryAtomicCounterBuffers = 0;
    Resources.maxFragmentAtomicCounterBuffers = 1;
    Resources.maxCombinedAtomicCounterBuffers = 1;
    Resources.maxAtomicCounterBufferSize = 16384;
    Resources.maxTransformFeedbackBuffers = 4;
    Resources.maxTransformFeedbackInterleavedComponents = 64;
    Resources.maxCullDistances = 8;
    Resources.maxCombinedClipAndCullDistances = 8;
    Resources.maxSamples = 4;
    Resources.maxMeshOutputVerticesNV = 256;
    Resources.maxMeshOutputPrimitivesNV = 512;
    Resources.maxMeshWorkGroupSizeX_NV = 32;
    Resources.maxMeshWorkGroupSizeY_NV = 1;
    Resources.maxMeshWorkGroupSizeZ_NV = 1;
    Resources.maxTaskWorkGroupSizeX_NV = 32;
    Resources.maxTaskWorkGroupSizeY_NV = 1;
    Resources.maxTaskWorkGroupSizeZ_NV = 1;
    Resources.maxMeshViewCountNV = 4;

    Resources.limits.nonInductiveForLoops = 1;
    Resources.limits.whileLoops = 1;
    Resources.limits.doWhileLoops = 1;
    Resources.limits.generalUniformIndexing = 1;
    Resources.limits.generalAttributeMatrixVectorIndexing = 1;
    Resources.limits.generalVaryingIndexing = 1;
    Resources.limits.generalSamplerIndexing = 1;
    Resources.limits.generalVariableIndexing = 1;
    Resources.limits.generalConstantMatrixVectorIndexing = 1;

    return Resources;
}

namespace v2
{

inline static void ShaderCompiler_ErrorHandler(glslang_shader_s* p_shader, glslang_program_t* p_program, const int32 p_return_code)
{
    if (!p_return_code)
    {
        if (p_shader)
        {
            printf(glslang_shader_get_info_log(p_shader));
            printf(glslang_shader_get_info_debug_log(p_shader));
        }
        if (p_program)
        {
            // glslang_shader_get_info_log()
            printf(glslang_program_get_info_log(p_program));
            printf(glslang_program_get_info_debug_log(p_program));
        }

        // glslang_shader_get_info_log()
        abort();
    }
};

inline static int8 ShaderCompiler_ErrorHandler_silent(glslang_shader_s* p_shader, glslang_program_t* p_program, const int32 p_return_code)
{
    if (!p_return_code)
    {
        if (p_shader)
        {
            printf(glslang_shader_get_info_log(p_shader));
            printf(glslang_shader_get_info_debug_log(p_shader));
        }
        if (p_program)
        {
            // glslang_shader_get_info_log()
            printf(glslang_program_get_info_log(p_program));
            printf(glslang_program_get_info_debug_log(p_program));
        }

        return 0;
    }
    return 1;
};

/*
    The ShaderCompiled allows runtime shader compiliation from text.
    /!\ For the actual game engine, it is more performant to store the compiled version of the shader.
*/
struct ShaderCompiled
{
    glslang_program_t* program;

    inline void free()
    {
        glslang_program_delete(this->program);
        this->program = NULL;
    };

    inline Slice<int8> get_compiled_binary()
    {
        return Slice<int8>::build_memory_elementnb((int8*)glslang_program_SPIRV_get_ptr(this->program), glslang_program_SPIRV_get_size(this->program) * sizeof(unsigned int));
    };
};

#define func_name def
#define ShaderCompiled_handle_error_slot sc_handle_error
#include "./shader_compiler_tt.hpp"

#define func_name silent
#define ShaderCompiled_handle_error_slot sc_handle_error_silent
#include "./shader_compiler_tt.hpp"

#define ShaderCompile_compile_def ShaderCompile_compile(def)
#define ShaderCompile_compile_silent ShaderCompile_compile(silent)

struct ShaderCompiler
{
    TBuiltInResource ressources;

    inline static ShaderCompiler allocate()
    {
        sc_handle_error(NULL, NULL, glslang_initialize_process());
        return ShaderCompiler{InitResources()};
    };

    inline void free()
    {
        glslang_finalize_process();
    };

    inline ShaderCompiled compile_shader(const ShaderModuleStage p_stage, const Slice<int8>& p_shader_string)
    {
        ShaderCompiled l_shader_compiled;
        ShaderCompile_compile_def(this->ressources, p_stage, p_shader_string, &l_shader_compiled);
        return l_shader_compiled;
    };

    inline int8 compile_shader_silent(const ShaderModuleStage p_stage, const Slice<int8>& p_shader_string, ShaderCompiled* out_shader_compiled)
    {
        return ShaderCompile_compile_silent(this->ressources, p_stage, p_shader_string, out_shader_compiled);
    };
};

} // namespace v2

#undef sc_handle_error
#undef sc_handle_error_silent