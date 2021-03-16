
#include "AssetCompiler/asset_compiler.hpp"

#define ASSETCOMPILER_API __declspec(dllexport)

ShaderCompiler g_shader_compiler;

enum class AssetCompilerAPIKey : uint32
{
    Initialize = 0,
    Finalize = 1,
    OpenDatabaseConnection = 2,
    CloseDatabaseConnection = 3,
    CompileAndPushToDabase = 4
};

inline void Initialize()
{
    g_shader_compiler = ShaderCompiler::allocate();
};

inline void Finalize()
{
    g_shader_compiler.free();
    memleak_ckeck();
};

inline void OpenDatabaseConnection(const Slice<void*>& p_args)
{
    Slice<int8> l_input_buffer = Slice<int8>::build_begin_end((int8*)p_args.get(0), 0, -1);
    BinaryDeserializer l_input_buffer_deserializer = BinaryDeserializer::build(l_input_buffer);
    Slice<int8> l_database_file_path = l_input_buffer_deserializer.slice();

    AssetDatabase* l_allocated_db = (AssetDatabase*)heap_malloc(sizeof(AssetDatabase));
    *l_allocated_db = AssetDatabase::allocate(l_database_file_path);
    *(AssetDatabase**)p_args.get(1) = l_allocated_db;
};

inline void CloseDatabaseConnection(const Slice<void*>& p_args)
{
    Slice<int8> l_input_buffer = Slice<int8>::build_begin_end((int8*)p_args.get(0), 0, -1);
    BinaryDeserializer l_input_buffer_deserializer = BinaryDeserializer::build(l_input_buffer);
    AssetDatabase* l_asset_database_token = *l_input_buffer_deserializer.type<AssetDatabase*>();
    l_asset_database_token->free();
    heap_free((int8*)l_asset_database_token);
};

inline void CompileAndPushToDabase(const Slice<void*>& p_args)
{
    Slice<int8> l_input_buffer = Slice<int8>::build_begin_end((int8*)p_args.get(0), 0, -1);
    BinaryDeserializer l_input_buffer_deserializer = BinaryDeserializer::build(l_input_buffer);
    AssetDatabase* l_asset_database = *l_input_buffer_deserializer.type<AssetDatabase*>();
    Slice<int8> l_root_path = l_input_buffer_deserializer.slice();
    Slice<int8> l_relative_asset_path = l_input_buffer_deserializer.slice();

    AssetCompiler_compile_and_push_to_database_single_file(g_shader_compiler, *l_asset_database, l_root_path, l_relative_asset_path);
};

extern "C"
{
    ASSETCOMPILER_API void __cdecl EntryPoint(void** argv, int argc)
    {
        AssetCompilerAPIKey l_key = *(AssetCompilerAPIKey*)argv[0];

        switch (l_key)
        {
        case AssetCompilerAPIKey::Initialize:
            Initialize();
            break;
        case AssetCompilerAPIKey::Finalize:
            Finalize();
            break;
        case AssetCompilerAPIKey::OpenDatabaseConnection:
            OpenDatabaseConnection(Slice<void*>::build_begin_end(argv, 1, argc));
            break;
        case AssetCompilerAPIKey::CloseDatabaseConnection:
            CloseDatabaseConnection(Slice<void*>::build_begin_end(argv, 1, argc));
            break;
        case AssetCompilerAPIKey::CompileAndPushToDabase:
            CompileAndPushToDabase(Slice<void*>::build_begin_end(argv, 1, argc));
            break;
        default:
            abort();
        };
    };
}