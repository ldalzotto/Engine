#include "AssetCompiler/asset_compiler.hpp"

#if 1
int main(int32 argc, int8** argv)
{
    if (argc > 1)
    {
        Slice<int8*> l_args = Slice<int8*>::build_memory_elementnb(argv, argc);
        if (l_args.Size >= 4)
        {
            Slice<int8> l_asset_database_path = slice_int8_build_rawstr(l_args.get(1));
            Slice<int8> l_asset_relative_path = slice_int8_build_rawstr(l_args.get(2));
            Slice<int8> l_asset_full_path = slice_int8_build_rawstr(l_args.get(3));

            ShaderCompiler l_shader_compiled = ShaderCompiler::allocate();
            AssetDatabase::initialize_database(l_asset_database_path);
            AssetDatabase l_asset_database = AssetDatabase::allocate(l_asset_database_path);

            File l_asset_file = File::open(l_asset_full_path);
            AssetCompiler_compile_and_push_to_database_single_file(l_shader_compiled, l_asset_database, l_asset_file, l_asset_relative_path);

            l_asset_file.free();
            l_asset_database.free();
            l_shader_compiled.free();
        }
    }

    memleak_ckeck();
}
#endif

#if 0
int main()
{
    ShaderCompiler l_shader_compiled = ShaderCompiler::allocate();
    File l_asset_file = File::open(slice_int8_build_rawstr("E:/GameProjects/GameEngineLinux/.tmp/QuadDraw.vert"));
    Span<int8> l_sp = AssetCompiler_compile_single_file(l_shader_compiled, l_asset_file);

    File l_target = File::create_or_open(slice_int8_build_rawstr("E:/GameProjects/GameEngineLinux/.tmp/QuadDrawVert.out"));
    l_target.write_file(l_sp.slice);
}
#endif