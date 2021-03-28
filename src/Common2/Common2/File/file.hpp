#pragma once

#if _WIN32
using FileHandle = HANDLE;
#endif

struct FileNative
{
    static FileHandle create_file(const Slice<int8>& p_path);

    static FileHandle open_file(const Slice<int8>& p_path);

    static void close_file(const FileHandle& p_file_handle);

    static void set_file_pointer(const FileHandle& p_file_handle, uimax p_pointer);

    static void delete_file(const Slice<int8>& p_path);

    static uimax get_file_size(const FileHandle& p_file_handle);

    static uimax get_modification_ts(const FileHandle& p_file_handle);

    inline static void read_buffer(const FileHandle& p_file_handle, Slice<int8>* in_out_buffer)
    {
#if __DEBUG
        assert_true(
#endif
            ReadFile(p_file_handle, in_out_buffer->Begin, (DWORD)in_out_buffer->Size, NULL, NULL)
#if __DEBUG
        )
#endif
            ;
    };

    inline static void write_buffer(const FileHandle& p_file_handle, const uimax p_offset, const Slice<int8>& p_buffer)
    {
#if __DEBUG
        assert_true(
#endif
            WriteFile(p_file_handle, p_buffer.Begin, (DWORD)p_buffer.Size, NULL, NULL)
#if __DEBUG
        )
#endif
            ;
    };

    static int8 handle_is_valid(const FileHandle& p_file_handle);
};

#if _WIN32

inline FileHandle FileNative::create_file(const Slice<int8>& p_path)
{
    FileHandle l_handle = CreateFile(p_path.Begin, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
#if __MEMLEAK
    if (FileNative::handle_is_valid(l_handle))
    {
        push_ptr_to_tracked((int8*)l_handle);
    }
#endif
    return l_handle;
};

inline FileHandle FileNative::open_file(const Slice<int8>& p_path)
{
    FileHandle l_handle = CreateFile(p_path.Begin, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
#if __MEMLEAK
    if (FileNative::handle_is_valid(l_handle))
    {
        push_ptr_to_tracked((int8*)l_handle);
    }
#endif
    return l_handle;
};

inline void FileNative::close_file(const FileHandle& p_file_handle)
{
#if __MEMLEAK
    if (FileNative::handle_is_valid(p_file_handle))
    {
        remove_ptr_to_tracked((int8*)p_file_handle);
    }
#endif

#if __DEBUG
    assert_true(
#endif
        CloseHandle(p_file_handle)
#if __DEBUG
    )
#endif
        ;
};

inline void FileNative::set_file_pointer(const FileHandle& p_file_handle, uimax p_pointer)
{
#if __DEBUG
    assert_true(
#endif
        SetFilePointer(p_file_handle, (LONG)p_pointer, 0, FILE_BEGIN)
#if __DEBUG
        != INVALID_SET_FILE_POINTER)
#endif
        ;
};

inline void FileNative::delete_file(const Slice<int8>& p_path)
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

inline uimax FileNative::get_file_size(const FileHandle& p_file_handle)
{
    DWORD l_file_size_high;
    DWORD l_return = GetFileSize(p_file_handle, &l_file_size_high);
#if __DEBUG
    assert_true(l_return != INVALID_FILE_SIZE);
#endif
    return dword_lowhigh_to_uimax(l_return, l_file_size_high);
};

inline uimax FileNative::get_modification_ts(const FileHandle& p_file_handle)
{
    FILETIME l_creation_time, l_last_access_time, l_last_write_time;
    BOOL l_filetime_return = GetFileTime(p_file_handle, &l_creation_time, &l_last_access_time, &l_last_write_time);
#if __DEBUG
    assert_true(l_filetime_return);
#endif
    uimax l_ct = FILETIME_to_mics(&l_creation_time);
    uimax l_at = FILETIME_to_mics(&l_last_access_time);
    uimax l_wt = FILETIME_to_mics(&l_last_write_time);

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

inline int8 FileNative::handle_is_valid(const FileHandle& p_file_handle)
{
    return p_file_handle != INVALID_HANDLE_VALUE;
};

#endif

struct File
{
    Slice<int8> path_slice;
    FileHandle native_handle;

    inline static File create(const Slice<int8>& p_path)
    {
        File l_file = create_silent(p_path);
#if __DEBUG
        assert_true(l_file.is_valid());
#endif
        return l_file;
    };

    inline static File open_silent(const Slice<int8>& p_path)
    {
        File l_file;
        l_file.path_slice = p_path;
        l_file.native_handle = FileNative::open_file(p_path);
        return l_file;
    };

    inline static File open(const Slice<int8>& p_path)
    {
        File l_file = open_silent(p_path);
#if __DEBUG
        assert_true(l_file.is_valid());
#endif
        return l_file;
    };

    inline static File create_or_open(const Slice<int8>& p_path)
    {
        File l_created_file = File::create_silent(p_path);
        if (!l_created_file.is_valid())
        {
            l_created_file = File::open_silent(p_path);
        }

        if (!l_created_file.is_valid())
        {
            abort();
        }

        return l_created_file;
    };

    inline void free()
    {
        FileNative::close_file(this->native_handle);
        this->native_handle = NULL;
    };

    inline void erase()
    {
        this->free();
        FileNative::delete_file(this->path_slice);
    };

    inline uimax get_size()
    {
        return FileNative::get_file_size(this->native_handle);
    };

    inline uimax get_modification_ts()
    {
        return FileNative::get_modification_ts(this->native_handle);
    };

    inline void read_file(Slice<int8>* in_out_buffer) const
    {
        FileNative::set_file_pointer(this->native_handle, 0);
        uimax l_file_size = FileNative::get_file_size(this->native_handle);
#if __DEBUG
        assert_true(l_file_size <= in_out_buffer->Size);
#endif
        in_out_buffer->Size = l_file_size;
        FileNative::read_buffer(this->native_handle, in_out_buffer);
    };

    inline Span<int8> read_file_allocate() const
    {
        uimax l_file_size = FileNative::get_file_size(this->native_handle);
        Span<int8> l_buffer = Span<int8>::allocate(l_file_size);
        this->read_file(&l_buffer.slice);
        return l_buffer;
    };

    inline void write_file(const Slice<int8>& p_buffer)
    {
        FileNative::set_file_pointer(this->native_handle, 0);
        FileNative::write_buffer(this->native_handle, 0, p_buffer);
    };

    inline int8 is_valid()
    {
        return FileNative::handle_is_valid(this->native_handle);
    };

  private:
    inline static File create_silent(const Slice<int8>& p_path)
    {
        File l_file;
        l_file.path_slice = p_path;
        l_file.native_handle = FileNative::create_file(p_path);
        return l_file;
    };

};
