#pragma once

inline String asset_database_test_initialize(const Slice<int8>& p_db_local_path)
{
	String l_database_path = String::allocate_elements(slice_int8_build_rawstr(ASSET_FOLDER_PATH));
	l_database_path.append(p_db_local_path);
	{
		File l_tmp_file = File::create_or_open(l_database_path.to_slice());
		l_tmp_file.erase_with_slicepath();
	}

	AssetDatabase::initialize_database(l_database_path.to_slice());
	return l_database_path;
};