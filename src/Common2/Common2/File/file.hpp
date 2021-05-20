#pragma once

// TODO -> there is a lot of duplication between windows and linux

struct FileHandle
{
#if __MEMLEAK
    int8* memleak_ptr;
#endif
#if _WIN32
    HANDLE handle;
#else
    int handle;
#endif
};

struct FileNative
{
    static FileHandle create_file(const Slice<int8>& p_path);

    static FileHandle open_file(const Slice<int8>& p_path);

    static void close_file(FileHandle& p_file_handle);

    static void set_file_pointer(const FileHandle& p_file_handle, uimax p_pointer);

    static void delete_file(const Slice<int8>& p_path);

    static uimax get_file_size(const FileHandle& p_file_handle);

    static uimax get_modification_ts(const FileHandle& p_file_handle);

    static void read_buffer(const FileHandle& p_file_handle, Slice<int8>* in_out_buffer);

    inline static void write_buffer(const FileHandle& p_file_handle, const uimax p_offset, const Slice<int8>& p_buffer);

    static int8 handle_is_valid(const FileHandle& p_file_handle);
};

#if _WIN32

inline FileHandle FileNative::create_file(const Slice<int8>& p_path)
{
    FileHandle l_handle;

    // We must do this check because the input path may not be null terminated and the win32 api CreateFile only allows null terminated string.
    // So if that's the case, we allocate a temporary string to be null terminated.
    if (p_path.get(p_path.Size - 1) != '\0' && p_path.Begin[p_path.Size] != '\0')
    {
        String l_file_path_null_terminated = String::allocate_elements(p_path);
        l_handle.handle =
            CreateFile(l_file_path_null_terminated.get_memory(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
        l_file_path_null_terminated.free();
    }
    else
    {
        l_handle.handle = CreateFile(p_path.Begin, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
    }

#if __MEMLEAK
    if (FileNative::handle_is_valid(l_handle))
    {
        l_handle.memleak_ptr = heap_malloc(1);
    }
#endif
    return l_handle;
};

inline FileHandle FileNative::open_file(const Slice<int8>& p_path)
{
    FileHandle l_handle;
    // We must do this check because the input path may not be null terminated and the win32 api CreateFile only allows null terminated string.
    // So if that's the case, we allocate a temporary string to be null terminated.
    if (p_path.get(p_path.Size - 1) != '\0' && p_path.Begin[p_path.Size] != '\0')
    {
        String l_file_path_null_terminated = String::allocate_elements(p_path);
        l_handle.handle = CreateFile(l_file_path_null_terminated.get_memory(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
                              FILE_ATTRIBUTE_NORMAL, NULL);
        l_file_path_null_terminated.free();
    }
    else
    {
        l_handle.handle = CreateFile(p_path.Begin, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    }

#if __MEMLEAK
    if (FileNative::handle_is_valid(l_handle))
    {
        l_handle.memleak_ptr = heap_malloc(1);
    }
#endif
    return l_handle;
};

inline void FileNative::close_file(FileHandle& p_file_handle)
{
#if __MEMLEAK
    if (FileNative::handle_is_valid(p_file_handle))
    {
        heap_free(p_file_handle.memleak_ptr);
    }
#endif

#if __DEBUG
    assert_true(
#endif
        CloseHandle(p_file_handle.handle)
#if __DEBUG
    )
#endif
        ;

    p_file_handle = FileHandle{0};
};

inline void FileNative::set_file_pointer(const FileHandle& p_file_handle, uimax p_pointer)
{
#if __DEBUG
    assert_true(
#endif
        SetFilePointer(p_file_handle.handle, (LONG)p_pointer, 0, FILE_BEGIN)
#if __DEBUG
        != INVALID_SET_FILE_POINTER)
#endif
        ;
};

inline void FileNative::delete_file(const Slice<int8>& p_path)
{
    // We must do this check because the input path may not be null terminated and the win32 api CreateFile only allows null terminated string.
    // So if that's the case, we allocate a temporary string to be null terminated.
    if (p_path.get(p_path.Size - 1) != '\0' && p_path.Begin[p_path.Size] != '\0')
    {
        String l_file_path_null_terminated = String::allocate_elements(p_path);
#if __DEBUG
        assert_true(
#endif
            DeleteFile(l_file_path_null_terminated.get_memory())
#if __DEBUG
        )
#endif
            ;
        l_file_path_null_terminated.free();
    }
    else
    {
#if __DEBUG
        assert_true(
#endif
            DeleteFile(p_path.Begin)
#if __DEBUG
        )
#endif
            ;
    }
};

inline uimax FileNative::get_file_size(const FileHandle& p_file_handle)
{
    DWORD l_file_size_high;
    DWORD l_return = GetFileSize(p_file_handle.handle, &l_file_size_high);
#if __DEBUG
    assert_true(l_return != INVALID_FILE_SIZE);
#endif
    return dword_lowhigh_to_uimax(l_return, l_file_size_high);
};

inline uimax FileNative::get_modification_ts(const FileHandle& p_file_handle)
{
    FILETIME l_creation_time, l_last_access_time, l_last_write_time;
    BOOL l_filetime_return = GetFileTime(p_file_handle.handle, &l_creation_time, &l_last_access_time, &l_last_write_time);
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

inline void FileNative::read_buffer(const FileHandle& p_file_handle, Slice<int8>* in_out_buffer)
{
#if __DEBUG
    assert_true(
#endif
        ReadFile(p_file_handle.handle, in_out_buffer->Begin, (DWORD)in_out_buffer->Size, NULL, NULL)
#if __DEBUG
    )
#endif
        ;
};

inline void FileNative::write_buffer(const FileHandle& p_file_handle, const uimax p_offset, const Slice<int8>& p_buffer)
{
#if __DEBUG
    assert_true(
#endif
        WriteFile(p_file_handle.handle, p_buffer.Begin, (DWORD)p_buffer.Size, NULL, NULL)
#if __DEBUG
    )
#endif
        ;
};

inline int8 FileNative::handle_is_valid(const FileHandle& p_file_handle)
{
    return p_file_handle.handle != INVALID_HANDLE_VALUE;
};

#else

#include <fcntl.h>
#include <sys/stat.h>

inline FileHandle FileNative::create_file(const Slice<int8>& p_path)
{
    FileHandle l_handle;

    // We must do this check because the input path may not be null terminated and the win32 api CreateFile only allows null terminated string.
    // So if that's the case, we allocate a temporary string to be null terminated.
    if (p_path.get(p_path.Size - 1) != '\0' && p_path.Begin[p_path.Size] != '\0')
    {
        String l_file_path_null_terminated = String::allocate_elements(p_path);
        l_handle.handle = open(l_file_path_null_terminated.get_memory(), O_RDWR | O_CREAT, S_IRWXU);
        l_file_path_null_terminated.free();
    }
    else
    {
        l_handle.handle = open(p_path.Begin, O_RDWR | O_CREAT, S_IRWXU);
    }

#if __MEMLEAK
    if (FileNative::handle_is_valid(l_handle))
    {
        l_handle.memleak_ptr = heap_malloc(1);
    }
#endif
    return l_handle;
};

inline FileHandle FileNative::open_file(const Slice<int8>& p_path)
{
    FileHandle l_handle;
    // We must do this check because the input path may not be null terminated and the win32 api CreateFile only allows null terminated string.
    // So if that's the case, we allocate a temporary string to be null terminated.
    if (p_path.get(p_path.Size - 1) != '\0' && p_path.Begin[p_path.Size] != '\0')
    {
        String l_file_path_null_terminated = String::allocate_elements(p_path);
        l_handle.handle = open(l_file_path_null_terminated.get_memory(), O_RDWR | O_CREAT, S_IRWXU);
        l_file_path_null_terminated.free();
    }
    else
    {
        l_handle.handle = open(p_path.Begin, O_RDWR | O_CREAT, S_IRWXU);
    }

#if __MEMLEAK
    if (FileNative::handle_is_valid(l_handle))
    {
        l_handle.memleak_ptr = heap_malloc(1);
    }
#endif
    return l_handle;
};

inline void FileNative::close_file(FileHandle& p_file_handle)
{
#if __MEMLEAK
    if (FileNative::handle_is_valid(p_file_handle))
    {
        heap_free(p_file_handle.memleak_ptr);
    }
#endif

#if __DEBUG
    assert_true(
#endif
        close(p_file_handle.handle)
#if __DEBUG
        == 0)
#endif
        ;

    p_file_handle = FileHandle{0};
};

inline void FileNative::set_file_pointer(const FileHandle& p_file_handle, uimax p_pointer)
{
#if __DEBUG
    assert_true(
#endif
        lseek(p_file_handle.handle, p_pointer, SEEK_SET)
#if __DEBUG
        == 0)
#endif
        ;
};

inline void FileNative::delete_file(const Slice<int8>& p_path)
{
    // We must do this check because the input path may not be null terminated and the win32 api CreateFile only allows null terminated string.
    // So if that's the case, we allocate a temporary string to be null terminated.
    if (p_path.get(p_path.Size - 1) != '\0' && p_path.Begin[p_path.Size] != '\0')
    {
        String l_file_path_null_terminated = String::allocate_elements(p_path);
#if __DEBUG
        assert_true(
#endif
            remove(l_file_path_null_terminated.get_memory())
#if __DEBUG
            == 0)
#endif
            ;
        l_file_path_null_terminated.free();
    }
    else
    {
#if __DEBUG
        assert_true(
#endif
            remove(p_path.Begin)
#if __DEBUG
            == 0)
#endif
            ;
    }
};

inline uimax FileNative::get_file_size(const FileHandle& p_file_handle)
{
    uint32 l_old_seek = lseek(p_file_handle.handle, 0, SEEK_CUR);
    uint32 l_size = lseek(p_file_handle.handle, 0, SEEK_END);
    lseek(p_file_handle.handle, l_old_seek, SEEK_SET);
#if __DEBUG
    assert_true(l_size != -1);
#endif
    return l_size;
};

inline uimax FileNative::get_modification_ts(const FileHandle& p_file_handle)
{
    struct stat l_stat;
    int l_ftat_return = fstat(p_file_handle.handle, &l_stat);
#if __DEBUG
    assert_true(l_ftat_return == 0);
#endif


    uimax l_ct = l_stat.st_atime;
    uimax l_at = l_stat.st_mtime;
    uimax l_wt = l_stat.st_ctime;

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

inline void FileNative::read_buffer(const FileHandle& p_file_handle, Slice<int8>* in_out_buffer)
{
#if __DEBUG
    assert_true(
#endif
        read(p_file_handle.handle, in_out_buffer->Begin, in_out_buffer->Size)
#if __DEBUG
    )
#endif
        ;
};

inline void FileNative::write_buffer(const FileHandle& p_file_handle, const uimax p_offset, const Slice<int8>& p_buffer)
{
#if __DEBUG
    assert_true(
#endif
        write(p_file_handle.handle, p_buffer.Begin, p_buffer.Size)
#if __DEBUG
    )
#endif
        ;
};

inline int8 FileNative::handle_is_valid(const FileHandle& p_file_handle)
{
    return p_file_handle.handle != -1;
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
