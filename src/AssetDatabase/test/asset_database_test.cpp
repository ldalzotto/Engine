#pragma once

#include "AssetDatabase/assetdatabase.hpp"
#include "asset_database_test_utils.hpp"

namespace v2
{
inline void asset_blob_insert_read_write()
{
    String l_database_path = asset_database_test_initialize(slice_int8_build_rawstr("asset.db"));
    AssetDatabase l_asset_database = AssetDatabase::allocate(l_database_path.to_slice());

    {
        Slice<int8> l_path = slice_int8_build_rawstr("pathtest");
        Slice<uimax> l_data = SliceN<uimax, 3>{0, 1, 2}.to_slice();
        hash_t l_inserted_id = l_asset_database.insert_asset_blob(l_path, l_data.build_asint8());
        Span<int8> l_retrieved_data = l_asset_database.get_asset_blob(l_inserted_id);
        assert_true(l_data.build_asint8().compare(l_retrieved_data.slice));
        l_retrieved_data.free();

        //insert_or_update_asset_blob -> doing update
        l_data = SliceN<uimax, 3>{3, 4, 5}.to_slice();
        l_inserted_id =  l_asset_database.insert_or_update_asset_blob(l_path, l_data.build_asint8());
        l_retrieved_data = l_asset_database.get_asset_blob(l_inserted_id);
        assert_true(l_data.build_asint8().compare(l_retrieved_data.slice));
        l_retrieved_data.free();
    }

    //insert_or_update_asset_blob -> doing insert
    {
        Slice<int8> l_path = slice_int8_build_rawstr("pathtest2");
        Slice<uimax> l_data = SliceN<uimax, 3>{0, 1, 2}.to_slice();
        hash_t l_inserted_id = l_asset_database.insert_or_update_asset_blob(l_path, l_data.build_asint8());
        Span<int8> l_retrieved_data = l_asset_database.get_asset_blob(l_inserted_id);
        assert_true(l_data.build_asint8().compare(l_retrieved_data.slice));
        l_retrieved_data.free();
    }

    l_asset_database.free();
    l_database_path.free();
};

inline void asset_dependencies_blob_read_write()
{
    String l_database_path = asset_database_test_initialize(slice_int8_build_rawstr("asset.db"));
    AssetDatabase l_asset_database = AssetDatabase::allocate(l_database_path.to_slice());
    // insert select
    {
        Slice<int8> l_path = slice_int8_build_rawstr("pathtest2");
        Slice<uimax> l_data = SliceN<uimax, 3>{0, 1, 2}.to_slice();
        hash_t l_inserted_id = l_asset_database.insert_asset_dependencies_blob(l_path, l_data.build_asint8());
        Span<int8> l_retrieved_data = l_asset_database.get_asset_dependencies_blob(l_inserted_id);
        assert_true(l_data.build_asint8().compare(l_retrieved_data.slice));
        l_retrieved_data.free();
    }
    l_asset_database.free();
    l_database_path.free();
};

} // namespace v2

int main()
{
    v2::asset_blob_insert_read_write();
    v2::asset_dependencies_blob_read_write();
}