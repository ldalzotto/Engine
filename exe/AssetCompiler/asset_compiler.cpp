#include "AssetCompiler/asset_compiler.hpp"

#if 1
// TODO -> this module must be removed in favor of the tool
int main(int32 argc, int8** argv)
{
    if (argc > 1)
    {
        Slice<int8*> l_args = Slice<int8*>::build_memory_elementnb(argv, argc);
        if (l_args.Size >= 4)
        {
            Slice<int8> l_asset_database_path = slice_int8_build_rawstr(l_args.get(1));
            Slice<int8> l_root_path = slice_int8_build_rawstr(l_args.get(2));
            Slice<int8> l_asset_relative_path = slice_int8_build_rawstr(l_args.get(3));

            ShaderCompiler l_shader_compiled = ShaderCompiler::allocate();
            DatabaseConnection l_database_connection = DatabaseConnection::allocate(l_asset_database_path);

            AssetDatabase::initialize_database(l_database_connection);
            AssetDatabase l_asset_database = AssetDatabase::allocate(l_database_connection);

            AssetMetadataDatabase::initialize_database(l_database_connection);
            AssetMetadataDatabase l_asset_metadata_database = AssetMetadataDatabase::allocate(l_database_connection);

            int8 l_compilation_result =
                AssetCompiler_compile_and_push_to_database_single_file(l_shader_compiled, l_database_connection, l_asset_database, l_asset_metadata_database, l_root_path, l_asset_relative_path);

            l_asset_metadata_database.free(l_database_connection);
            l_asset_database.free(l_database_connection);
            l_database_connection.free();
            l_shader_compiled.free();

            if (!l_compilation_result)
            {
                abort();
            }
        }
    }

    memleak_ckeck();
}
#endif
