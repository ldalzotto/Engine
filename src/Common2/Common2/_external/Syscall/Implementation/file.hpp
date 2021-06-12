#pragma once

file_native file_native_create_file_unchecked(const Slice<int8>& p_path)
{
    file_native l_file;
    l_file.ptr = (uimax)CreateFile(p_path.Begin, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
    return l_file;
};

file_native file_native_open_file_unchecked(const Slice<int8>& p_path)
{
    file_native l_file;
    l_file.ptr = (uimax)CreateFile(p_path.Begin, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    return l_file;
};

void file_native_close_file_unchecked(file_native p_file_handle)
{
#if __DEBUG
    assert_true(
#endif
        CloseHandle((HANDLE)p_file_handle.ptr)
#if __DEBUG
    )
#endif
        ;
};

void file_native_delete_file_unchecked(const Slice<int8>& p_path)
{
#if __DEBUG
    assert_true(
#endif
        DeleteFile(p_path.Begin)
#if __DEBUG
    )
#endif
        ;
};

void file_native_set_file_pointer(file_native p_file_handle, uimax p_pointer)
{
#if __DEBUG
    assert_true(
#endif
        SetFilePointer((HANDLE)p_file_handle.ptr, (LONG)p_pointer, 0, FILE_BEGIN)
#if __DEBUG
        != INVALID_SET_FILE_POINTER)
#endif
        ;
};

uimax file_native_get_file_size(file_native p_file_handle)
{
    DWORD l_file_size_high;
    DWORD l_return = GetFileSize((HANDLE)p_file_handle.ptr, &l_file_size_high);
#if __DEBUG
    assert_true(l_return != INVALID_FILE_SIZE);
#endif
    return dword_lowhigh_to_uimax(l_return, l_file_size_high);
};

uimax file_native_get_modification_ts(file_native p_file_handle)
{
    FILETIME l_creation_time, l_last_access_time, l_last_write_time;
    BOOL l_filetime_return = GetFileTime((HANDLE)p_file_handle.ptr, &l_creation_time, &l_last_access_time, &l_last_write_time);
#if __DEBUG
    assert_true(l_filetime_return);
#endif
    uimax l_ct = FILETIME_to_mics(l_creation_time);
    uimax l_at = FILETIME_to_mics(l_last_access_time);
    uimax l_wt = FILETIME_to_mics(l_last_write_time);

    uimax l_return = l_ct;
    if (l_at >= l_return)
    {
        l_return = l_at;
    }
    if (l_wt >= l_return)
    {
        l_return = l_wt;
    }
    return l_return;
};

void file_native_read_buffer(file_native p_file_handle, Slice<int8>* in_out_buffer)
{
#if __DEBUG
    assert_true(
#endif
        ReadFile((HANDLE)p_file_handle.ptr, in_out_buffer->Begin, (DWORD)in_out_buffer->Size, NULL, NULL)
#if __DEBUG
    )
#endif
        ;
};

void file_native_write_buffer(file_native p_file_handle, const uimax p_offset, const Slice<int8>& p_buffer)
{
#if __DEBUG
    assert_true(
#endif
        WriteFile((HANDLE)p_file_handle.ptr, p_buffer.Begin, (DWORD)p_buffer.Size, NULL, NULL)
#if __DEBUG
    )
#endif
        ;
};

int8 file_native_handle_is_valid(file_native p_file_handle)
{
    return (void*)p_file_handle.ptr != INVALID_HANDLE_VALUE;
};