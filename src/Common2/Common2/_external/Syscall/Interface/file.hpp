#pragma once

struct file_native
{
    uimax ptr;
};


file_native file_native_create_file_unchecked(const Slice<int8>& p_path);
file_native file_native_open_file_unchecked(const Slice<int8>& p_path);
void file_native_close_file_unchecked(file_native p_file_handle);
void file_native_delete_file_unchecked(const Slice<int8>& p_path);
void file_native_set_file_pointer(file_native p_file_handle, uimax p_pointer);
uimax file_native_get_file_size(file_native p_file_handle);
uimax file_native_get_modification_ts(file_native p_file_handle);
void file_native_read_buffer(file_native p_file_handle, Slice<int8>* in_out_buffer);
void file_native_write_buffer(file_native p_file_handle, const uimax p_offset, const Slice<int8>& p_buffer);
int8 file_native_handle_is_valid(file_native p_file_handle);