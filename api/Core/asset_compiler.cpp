#include "AssetCompiler/asset_compiler.hpp"

#define EXPORT __declspec(dllexport)

enum class AssetCompilerFunctions : int8
{
    DatabaseConnection_Allocate = 0,
    DatabaseConnection_Free = 1,

    AssetMetadataDatabase_Allocate = 10,
    AssetMetadataDatabase_Free = 11,
    AssetMetadataDatabase_GetPaths = 12
};

extern "C"
{
    EXPORT inline void EntryPoint(const Slice<int8>& p_input, VectorSlice<int8>& out)
    {
        BinaryDeserializer l_deserializer = BinaryDeserializer::build(p_input);
        AssetCompilerFunctions l_function = *l_deserializer.type<AssetCompilerFunctions>();
        switch (l_function)
        {
        case AssetCompilerFunctions::DatabaseConnection_Allocate:
        {
            Slice<int8> l_file_path = l_deserializer.slice();
            DatabaseConnection* l_connection = (DatabaseConnection*)heap_malloc(sizeof(DatabaseConnection));
            *l_connection = DatabaseConnection::allocate(l_file_path);
            out.push_back_array(Slice<DatabaseConnection*>::build_asint8_memory_singleelement(&l_connection).build_asint8());
        }
        break;
        case AssetCompilerFunctions::DatabaseConnection_Free:
        {
            DatabaseConnection* l_database_connection = *l_deserializer.type<DatabaseConnection*>();
            l_database_connection->free();
            heap_free((int8*)l_database_connection);
        }
        break;
        case AssetCompilerFunctions::AssetMetadataDatabase_Allocate:
        {
            DatabaseConnection* l_database_connection = *l_deserializer.type<DatabaseConnection*>();
            AssetMetadataDatabase* l_asset_metadata_database = (AssetMetadataDatabase*)heap_malloc(sizeof(AssetMetadataDatabase));
            *l_asset_metadata_database = AssetMetadataDatabase::allocate(*l_database_connection);
            out.push_back_array(Slice<AssetMetadataDatabase*>::build_asint8_memory_singleelement(&l_asset_metadata_database).build_asint8());
        }
        break;
        case AssetCompilerFunctions::AssetMetadataDatabase_Free:
        {
            DatabaseConnection* l_database_connection = *l_deserializer.type<DatabaseConnection*>();
            AssetMetadataDatabase* l_asset_metadata_database = *l_deserializer.type<AssetMetadataDatabase*>();
            l_asset_metadata_database->free(*l_database_connection);
            heap_free((int8*)l_asset_metadata_database);
        }
        break;
        case AssetCompilerFunctions::AssetMetadataDatabase_GetPaths:
        {
            DatabaseConnection* l_database_connection = *l_deserializer.type<DatabaseConnection*>();
            AssetMetadataDatabase* l_asset_metadata_database = *l_deserializer.type<AssetMetadataDatabase*>();
            Slice<int8> l_asset_type = l_deserializer.slice();
            AssetMetadataDatabase::Paths l_assets_path = l_asset_metadata_database->get_all_path_from_type(*l_database_connection, l_asset_type);
            iVector<VectorSlice<int8>> l_out = iVector<VectorSlice<int8>>{out};
            BinarySerializer::varying_slice(l_out, l_assets_path.data_v2.varying_vector.to_varying_slice());
            l_assets_path.free();
        }
        break;
        }
    };

    EXPORT inline void Finalize()
    {
        memleak_ckeck();
    };
}
