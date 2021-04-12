#pragma once

#include "AssetCompiler/asset_compiler.hpp"

int32 main(int32 argc, int8** argv)
{
    assert_true(argc >= 3);

    Span<Slice<int8>> l_args = Span<Slice<int8>>::allocate(argc);
    for (loop(i, 0, l_args.Capacity))
    {
        l_args.get(i) = slice_int8_build_rawstr(argv[i]);
    }

    AssetCompilerPassHeap l_heap = AssetCompilerPassHeap::allocate();
    Vector<Token<AssetCompilationPass>> l_passes = Vector<Token<AssetCompilationPass>>::allocate(0);
    AssetCompilerPassComposition::allocate_passes_from_json_configuration(l_heap, l_args.get(1), l_args.get(2), &l_passes);

    ShaderCompiler l_shader_compiler = ShaderCompiler::allocate();

    for (loop(i, 0, l_passes.Size))
    {
        AssetCompilerPassComposition::ExecutionState l_exec = AssetCompilerPassComposition::ExecutionState::allocate(l_shader_compiler, l_heap.asset_compilation_passes.get(l_passes.get(i)));
        for (loop(j, 0, l_exec.get_size()))
        {
            int8 l_compilation = l_exec.compile_single(j);
            if (!l_compilation)
            {
                printf("ERROR | root : ");
                printf(l_exec.asset_compilation_pass.root_path.to_slice().Begin);
                printf(" | database : ");
                printf(l_exec.asset_compilation_pass.database_path.to_slice().Begin);
                printf(" | asset : ");
                printf(l_exec.asset_compilation_pass.assets_to_compile.get(j).to_slice().Begin);
                printf("\n");
            }
        }
        l_heap.free_asset_compiler_pass(l_passes.get(i));
    }

    l_passes.free();
    l_heap.free();
    l_shader_compiler.free();
    l_args.free();
};