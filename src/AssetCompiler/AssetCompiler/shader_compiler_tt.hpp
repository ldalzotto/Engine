
#ifndef func_name
#error func_name
#endif

#ifndef ShaderCompiled_handle_error_slot
#error ShaderCompiled_handle_error_slot
#endif

inline int8 func_name(const TBuiltInResource& p_ressources, const ShaderModuleStage p_stage, const Slice<int8>& p_shader_string, ShaderCompiled* out_shader_compiled)
{
    out_shader_compiled->program = NULL;

    glslang_input_t input = {GLSLANG_SOURCE_GLSL,
                             GLSLANG_STAGE_VERTEX,
                             GLSLANG_CLIENT_VULKAN,
                             GLSLANG_TARGET_VULKAN_1_2,
                             GLSLANG_TARGET_SPV,
                             GLSLANG_TARGET_SPV_1_0,
                             p_shader_string.Begin,
                             450,
                             GLSLANG_NO_PROFILE,
                             0,
                             0,
                             GLSLANG_MSG_DEFAULT_BIT,
                             (const glslang_resource_t*)&p_ressources};

    if (p_stage == ShaderModuleStage::VERTEX)
    {
        input.stage = GLSLANG_STAGE_VERTEX;
    }
    else if (p_stage == ShaderModuleStage::FRAGMENT)
    {
        input.stage = GLSLANG_STAGE_FRAGMENT;
    }

    glslang_shader_s* l_shader = glslang_shader_create(&input);
    ShaderCompiled_handle_error_slot(out_shader_compiled->program, NULL, glslang_shader_preprocess(l_shader, &input));
    ShaderCompiled_handle_error_slot(out_shader_compiled->program, l_shader, glslang_shader_parse(l_shader, &input));

    glslang_program_t* l_program = glslang_program_create();
    glslang_program_add_shader(l_program, l_shader);
    ShaderCompiled_handle_error_slot(out_shader_compiled->program, l_shader, glslang_program_link(l_program, GLSLANG_MSG_SPV_RULES_BIT | GLSLANG_MSG_VULKAN_RULES_BIT));
    glslang_program_SPIRV_generate(l_program, input.stage);
    if (glslang_program_SPIRV_get_messages(l_program))
    {
        printf("%s", glslang_program_SPIRV_get_messages(l_program));
    }

    glslang_shader_delete(l_shader);

    out_shader_compiled->program = l_program;
    return 1;
};

#undef func_name
#undef ShaderCompiled_handle_error_slot