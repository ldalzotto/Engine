
#include "AssetCompiler/asset_compiler.hpp"

int main()
{

    ShaderCompiler l_shader_compiler = ShaderCompiler::allocate();
    File l_file = File::open(slice_int8_build_rawstr("E:/GameProjects/GameEngineLinux/src/AssetCompiler/test/.asset/shad.vert"));
    Span<int8> l_compiled_asset = AssetCompiler_compile_single_file(l_shader_compiler, l_file);
    l_compiled_asset.free();
    l_file.free();
    l_shader_compiler.free();

    memleak_ckeck();
}