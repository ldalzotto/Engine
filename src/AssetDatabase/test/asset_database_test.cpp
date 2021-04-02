#include "AssetDatabase/assetdatabase.hpp"
#include "asset_database_test_utils.hpp"

inline void asset_blob_insert_read_write()
{
    String l_database_path = asset_database_test_initialize(Slice_int8_build_rawstr("asset.db"));
    DatabaseConnection l_connection = DatabaseConnection::allocate(l_database_path.to_slice());
    AssetDatabase l_asset_database = AssetDatabase::allocate(l_connection);

    int l_test[4] = {0, 1, 4, 2};
    {
        Slice<int8> l_path = Slice_int8_build_rawstr("pathtest");
        {
            Slice_declare_sized(uimax, 3, l_data_arr, l_data, 0, 1, 2);
            Slice<int8> l_data_int8 = Slice_build_asint8(&l_data);
            hash_t l_inserted_id = l_asset_database.insert_asset_blob(l_connection, l_path, Slice_build_asint8(&l_data));
            Span<int8> l_retrieved_data = l_asset_database.get_asset_blob(l_connection, l_inserted_id);
            assert_true(Slice_compare(&l_data_int8, &l_retrieved_data.slice));
            Span_free(&l_retrieved_data);
        }
        {
            // insert_or_update_asset_blob -> doing update
            Slice_declare_sized(uimax, 3, l_data_arr, l_data, 3, 4, 5);
            Slice<int8> l_data_int8 = Slice_build_asint8(&l_data);
            hash_t l_inserted_id = l_asset_database.insert_or_update_asset_blob(l_connection, l_path, Slice_build_asint8(&l_data));
            Span<int8> l_retrieved_data = l_asset_database.get_asset_blob(l_connection, l_inserted_id);
            assert_true(Slice_compare(&l_data_int8, &l_retrieved_data.slice));
            Span_free(&l_retrieved_data);
        }
    }

    // insert_or_update_asset_blob -> doing insert
    {
        Slice<int8> l_path = Slice_int8_build_rawstr("pathtest2");
        Slice_declare_sized(uimax, 3, l_data_arr, l_data, 0, 1, 2);
        Slice<int8> l_data_int8 = Slice_build_asint8(&l_data);
        hash_t l_inserted_id = l_asset_database.insert_or_update_asset_blob(l_connection, l_path, Slice_build_asint8(&l_data));
        Span<int8> l_retrieved_data = l_asset_database.get_asset_blob(l_connection, l_inserted_id);
        assert_true(Slice_compare(&l_data_int8, &l_retrieved_data.slice));
        Span_free(&l_retrieved_data);
    }

    l_asset_database.free(l_connection);
    l_connection.free();
    l_database_path.free();
};

inline void asset_dependencies_blob_read_write()
{
    String l_database_path = asset_database_test_initialize(Slice_int8_build_rawstr("asset.db"));
    DatabaseConnection l_connection = DatabaseConnection::allocate(l_database_path.to_slice());
    AssetDatabase l_asset_database = AssetDatabase::allocate(l_connection);
    // insert select
    {
        Slice<int8> l_path = Slice_int8_build_rawstr("pathtest2");
        Slice_declare_sized(uimax, 3, l_data_arr, l_data, 0, 1, 2);
        Slice<int8> l_data_int8 = Slice_build_asint8(&l_data);
        hash_t l_inserted_id = HashSlice(l_path);
        l_asset_database.insert_asset_dependencies_blob(l_connection, l_path, Slice_build_asint8(&l_data));
        Span<int8> l_retrieved_data = l_asset_database.get_asset_dependencies_blob(l_connection, l_inserted_id);
        assert_true(Slice_compare(&l_data_int8, &l_retrieved_data.slice));
        Span_free(&l_retrieved_data);
    }
    l_asset_database.free(l_connection);
    l_connection.free();
    l_database_path.free();
};

int main()
{
    asset_blob_insert_read_write();
    asset_dependencies_blob_read_write();
}