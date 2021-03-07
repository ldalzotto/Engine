
#include "AssetCompiler/asset_compiler.hpp"
#include "asset_database_test_utils.hpp"

inline void shader_asset_compilation(ShaderCompiler& p_shader_compiler)
{
    String l_asset_database_path = asset_database_test_initialize(slice_int8_build_rawstr("asset.db"));
    AssetDatabase l_asset_database = AssetDatabase::allocate(l_asset_database_path.to_slice());

    Span<int8> l_asset_root_path = Span<int8>::allocate_slice_2(slice_int8_build_rawstr(ASSET_FOLDER_PATH), slice_int8_build_rawstr("asset/"));

    AssetCompiler_compile_and_push_to_database_single_file(p_shader_compiler, l_asset_database, l_asset_root_path.slice, slice_int8_build_rawstr("shader_asset_test.json"));

    Span<int8> l_shader_ressource_compiled = l_asset_database.get_asset_blob(HashSlice(slice_int8_build_rawstr("shader_asset_test.json")));

    {
        v2::ShaderRessource::Asset::Value l_shader_value = v2::ShaderRessource::Asset::Value::build_from_asset(v2::ShaderRessource::Asset::build_from_binary(l_shader_ressource_compiled));

        assert_true(l_shader_value.execution_order == 1001);
        assert_true(l_shader_value.shader_configuration.zwrite == 0);
        assert_true(l_shader_value.shader_configuration.ztest == v2::ShaderConfiguration::CompareOp::Always);
        assert_true(l_shader_value.specific_parameters.compare(SliceN<v2::ShaderLayoutParameterType, 1>{v2::ShaderLayoutParameterType::TEXTURE_FRAGMENT}.to_slice()));

        l_shader_ressource_compiled.free();
    }
    {
        // TODO ->
        // v2::ShaderRessource::AssetDependencies::Value l_shader_dependencies = v2::ShaderRessource::AssetDependencies::Value::
    }

    l_asset_root_path.free();
    l_asset_database.free();
    l_asset_database_path.free();
};

int main()
{
    ShaderCompiler l_shader_compiler = ShaderCompiler::allocate();

    shader_asset_compilation(l_shader_compiler);

#if 0
    // File l_file = File::open(slice_int8_build_rawstr("E:/GameProjects/GameEngineLinux/src/AssetCompiler/test/.asset/shad.vert"));
    File l_file = File::open(slice_int8_build_rawstr("E:/GameProjects/GameEngineLinux/src/AssetCompiler/test/.asset/shader_asset_test.json"));

    Span<int8> l_compiled_asset = AssetCompiler_compile_single_file(l_shader_compiler, l_file);

    l_compiled_asset.free();
    l_file.free();
#endif
    l_shader_compiler.free();

    memleak_ckeck();
}