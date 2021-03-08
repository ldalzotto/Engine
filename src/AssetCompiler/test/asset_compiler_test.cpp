
#include "AssetCompiler/asset_compiler.hpp"
#include "asset_database_test_utils.hpp"

inline void shader_module_compilation(ShaderCompiler& p_shader_compiler)
{
    String l_asset_database_path = asset_database_test_initialize(slice_int8_build_rawstr("asset.db"));
    AssetDatabase l_asset_database = AssetDatabase::allocate(l_asset_database_path.to_slice());
    Span<int8> l_asset_root_path = Span<int8>::allocate_slice_2(slice_int8_build_rawstr(ASSET_FOLDER_PATH), slice_int8_build_rawstr("asset/"));

    AssetCompiler_compile_and_push_to_database_single_file(p_shader_compiler, l_asset_database, l_asset_root_path.slice, slice_int8_build_rawstr("shad.frag"));

    {
        Span<int8> l_raw_file = AssetCompiler_open_and_read_asset_file(l_asset_root_path.slice, slice_int8_build_rawstr("shad.frag"));
        l_raw_file.get(l_raw_file.Capacity - 1) = (int8)NULL;
        ShaderCompiled l_shader = p_shader_compiler.compile_shader(ShaderModuleStage::FRAGMENT, l_raw_file.slice);

        l_raw_file.free();

        Span<int8> l_shader_module_compiled = l_asset_database.get_asset_blob(HashSlice(slice_int8_build_rawstr("shad.frag")));

        assert_true(l_shader_module_compiled.slice.compare(l_shader.get_compiled_binary()));

        l_shader_module_compiled.free();
        l_shader.free();
    }

    l_asset_root_path.free();
    l_asset_database.free();
    l_asset_database_path.free();
};

inline void shader_asset_compilation(ShaderCompiler& p_shader_compiler)
{
    String l_asset_database_path = asset_database_test_initialize(slice_int8_build_rawstr("asset.db"));
    AssetDatabase l_asset_database = AssetDatabase::allocate(l_asset_database_path.to_slice());

    Span<int8> l_asset_root_path = Span<int8>::allocate_slice_2(slice_int8_build_rawstr(ASSET_FOLDER_PATH), slice_int8_build_rawstr("asset/"));

    AssetCompiler_compile_and_push_to_database_single_file(p_shader_compiler, l_asset_database, l_asset_root_path.slice, slice_int8_build_rawstr("shader_asset_test.json"));

    {
        Span<int8> l_shader_ressource_compiled = l_asset_database.get_asset_blob(HashSlice(slice_int8_build_rawstr("shader_asset_test.json")));
        v2::ShaderRessource::Asset::Value l_shader_value = v2::ShaderRessource::Asset::Value::build_from_asset(v2::ShaderRessource::Asset::build_from_binary(l_shader_ressource_compiled));

        assert_true(l_shader_value.execution_order == 1001);
        assert_true(l_shader_value.shader_configuration.zwrite == 0);
        assert_true(l_shader_value.shader_configuration.ztest == v2::ShaderConfiguration::CompareOp::Always);
        assert_true(l_shader_value.specific_parameters.compare(SliceN<v2::ShaderLayoutParameterType, 1>{v2::ShaderLayoutParameterType::TEXTURE_FRAGMENT}.to_slice()));

        l_shader_ressource_compiled.free();
    }
    {
        Span<int8> l_shader_dependencies_compiled = l_asset_database.get_asset_dependencies_blob(HashSlice(slice_int8_build_rawstr("shader_asset_test.json")));
        v2::ShaderRessource::AssetDependencies::Value l_shader_dependencies =
            v2::ShaderRessource::AssetDependencies::Value::build_from_asset(v2::ShaderRessource::AssetDependencies{l_shader_dependencies_compiled});

        assert_true(l_shader_dependencies.vertex_module == HashSlice(slice_int8_build_rawstr("shad.vert")));
        assert_true(l_shader_dependencies.fragment_module == HashSlice(slice_int8_build_rawstr("shad.frag")));

        l_shader_dependencies_compiled.free();
    }

    l_asset_root_path.free();
    l_asset_database.free();
    l_asset_database_path.free();
};

inline void material_asset_compilation(ShaderCompiler& p_shader_compiler)
{
    String l_asset_database_path = asset_database_test_initialize(slice_int8_build_rawstr("asset.db"));
    AssetDatabase l_asset_database = AssetDatabase::allocate(l_asset_database_path.to_slice());

    Span<int8> l_asset_root_path = Span<int8>::allocate_slice_2(slice_int8_build_rawstr(ASSET_FOLDER_PATH), slice_int8_build_rawstr("asset/"));

    AssetCompiler_compile_and_push_to_database_single_file(p_shader_compiler, l_asset_database, l_asset_root_path.slice, slice_int8_build_rawstr("material_asset_test.json"));

    {
        Span<int8> l_material_ressource_compiled = l_asset_database.get_asset_blob(HashSlice(slice_int8_build_rawstr("material_asset_test.json")));
        v2::MaterialRessource::Asset::Value l_material_value = v2::MaterialRessource::Asset::Value::build_from_asset(v2::MaterialRessource::Asset::build_from_binary(l_material_ressource_compiled));

        assert_true(l_material_value.parameters.get_size() == 1);
        Slice<int8> l_parameter = l_material_value.parameters.get_element(0);
        assert_true(*(ShaderParameter::Type*)l_parameter.Begin == ShaderParameter::Type::TEXTURE_GPU);
        l_parameter.slide(sizeof(ShaderParameter::Type));
        hash_t l_texture_hash = HashSlice(slice_int8_build_rawstr("link_to_png.png"));
        assert_true(l_parameter.compare(Slice<hash_t>::build_asint8_memory_singleelement(&l_texture_hash)));

        l_material_ressource_compiled.free();
    }
    {
        Span<int8> l_material_dependencies_compiled = l_asset_database.get_asset_dependencies_blob(HashSlice(slice_int8_build_rawstr("material_asset_test.json")));
        v2::MaterialRessource::AssetDependencies::Value l_material_dependencies =
            v2::MaterialRessource::AssetDependencies::Value::build_from_asset(v2::MaterialRessource::AssetDependencies{l_material_dependencies_compiled});

        assert_true(l_material_dependencies.shader == HashSlice(slice_int8_build_rawstr("shader_asset_test.json")));
        assert_true(l_material_dependencies.textures.Size == 1);
        assert_true(l_material_dependencies.textures.get(0) == HashSlice(slice_int8_build_rawstr("link_to_png.png")));

        l_material_dependencies_compiled.free();
    }

    l_asset_root_path.free();
    l_asset_database.free();
    l_asset_database_path.free();
};

int main()
{
    ShaderCompiler l_shader_compiler = ShaderCompiler::allocate();

    shader_module_compilation(l_shader_compiler);
    shader_asset_compilation(l_shader_compiler);
    material_asset_compilation(l_shader_compiler);

    l_shader_compiler.free();

    memleak_ckeck();
}