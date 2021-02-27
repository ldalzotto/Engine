#pragma once

#include "AssetDatabase/assetdatabase.hpp"

namespace v2
{
	inline void asset_blob_insert_read()
	{
		String l_database_path = String::allocate_elements(slice_int8_build_rawstr(ASSET_FOLDER_PATH));
		l_database_path.append(slice_int8_build_rawstr("asset.db"));
		{
			File l_tmp_file = File::create_or_open(l_database_path.to_slice());
			l_tmp_file.erase_with_slicepath();
		}

		AssetDatabase::initialize_database(l_database_path.to_slice());
		AssetDatabase l_asset_database = AssetDatabase::allocate(l_database_path.to_slice());

		{
			Slice<int8> l_path = slice_int8_build_rawstr("pathtest");
			Slice<uimax> l_data = SliceN<uimax, 3>{ 0, 1, 2 }.to_slice();
			hash_t l_inserted_id = l_asset_database.insert_asset_blob(l_path, l_data.build_asint8());
			Span<int8> l_retrieved_data = l_asset_database.get_asset_blob(l_inserted_id);
			assert_true(l_data.build_asint8().compare(l_retrieved_data.slice));
			l_retrieved_data.free();
		}

		l_asset_database.free();
		l_database_path.free();
	};
}


int main()
{
	v2::asset_blob_insert_read();
}