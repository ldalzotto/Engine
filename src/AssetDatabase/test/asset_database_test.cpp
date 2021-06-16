#include "AssetDatabase/assetdatabase.hpp"
#include "asset_database_test_utils.hpp"

inline void asset_blob_insert_read_write()
{
    String l_database_path = asset_database_test_initialize(slice_int8_build_rawstr("asset.db"));
    DatabaseConnection l_connection = DatabaseConnection::allocate(l_database_path.to_slice());
    AssetDatabase l_asset_database = AssetDatabase::allocate(l_connection);

    {
        Slice<int8> l_path = slice_int8_build_rawstr("pathtest");
        SliceN<uimax, 3> l_data_arr{0, 1, 2};
        Slice<uimax> l_data = slice_from_slicen(&l_data_arr);
        hash_t l_inserted_id = l_asset_database.insert_asset_blob(l_connection, l_path, l_data.build_asint8());
        Span<int8> l_retrieved_data = l_asset_database.get_asset_blob(l_connection, l_inserted_id);
        assert_true(l_data.build_asint8().compare(l_retrieved_data.slice));
        l_retrieved_data.free();

        //insert_or_update_asset_blob -> doing update
        l_data_arr= SliceN<uimax, 3>{3, 4, 5};
        l_data = slice_from_slicen(&l_data_arr);
        l_inserted_id =  l_asset_database.insert_or_update_asset_blob(l_connection, l_path, l_data.build_asint8());
        l_retrieved_data = l_asset_database.get_asset_blob(l_connection, l_inserted_id);
        assert_true(l_data.build_asint8().compare(l_retrieved_data.slice));
        l_retrieved_data.free();
    }

    //insert_or_update_asset_blob -> doing insert
    {
        Slice<int8> l_path = slice_int8_build_rawstr("pathtest2");
        SliceN<uimax, 3> l_data_arr{0, 1, 2};
        Slice<uimax> l_data = slice_from_slicen(&l_data_arr);
        hash_t l_inserted_id = l_asset_database.insert_or_update_asset_blob(l_connection, l_path, l_data.build_asint8());
        Span<int8> l_retrieved_data = l_asset_database.get_asset_blob(l_connection, l_inserted_id);
        assert_true(l_data.build_asint8().compare(l_retrieved_data.slice));
        l_retrieved_data.free();
    }

    l_asset_database.free(l_connection);
    l_connection.free();
    l_database_path.free();
};

inline void asset_dependencies_blob_read_write()
{
    String l_database_path = asset_database_test_initialize(slice_int8_build_rawstr("asset.db"));
    DatabaseConnection l_connection = DatabaseConnection::allocate(l_database_path.to_slice());
    AssetDatabase l_asset_database = AssetDatabase::allocate(l_connection);
    // insert select
    {
        Slice<int8> l_path = slice_int8_build_rawstr("pathtest2");
        SliceN<uimax, 3> l_data_arr{0, 1, 2};
        Slice<uimax> l_data = slice_from_slicen(&l_data_arr);
        hash_t l_inserted_id = HashFunctions::hash(l_path);
        l_asset_database.insert_asset_dependencies_blob(l_connection, l_path, l_data.build_asint8());
        Span<int8> l_retrieved_data = l_asset_database.get_asset_dependencies_blob(l_connection, l_inserted_id);
        assert_true(l_data.build_asint8().compare(l_retrieved_data.slice));
        l_retrieved_data.free();
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

#include "Common2/common2_external_implementation.hpp"