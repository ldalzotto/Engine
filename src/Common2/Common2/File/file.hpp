#pragma once

#if _WIN32
using FileHandle = HANDLE;
#endif

struct FileNative
{
    static FileHandle create_file(const Slice<int8>& p_path);

    static FileHandle open_file(const Slice<int8>& p_path);

    static void close_file(const FileHandle& p_file_handle);

    static void set_file_pointer(FileHandle& p_file_handle, uimax p_pointer);

    static void delete_file(const Slice<int8>& p_path);

    static int8 handle_is_valid(const FileHandle& p_file_handle);
};

#if _WIN32

inline FileHandle FileNative::create_file(const Slice<int8>& p_path)
{
    FileHandle l_handle = CreateFile(p_path.Begin, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
#if MEM_LEAK_DETECTION
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
#if MEM_LEAK_DETECTION
    if (FileNative::handle_is_valid(l_handle))
    {
        push_ptr_to_tracked((int8*)l_handle);
    }
#endif
    return l_handle;
};

inline void FileNative::close_file(const FileHandle& p_file_handle)
{
#if MEM_LEAK_DETECTION
    if (FileNative::handle_is_valid(p_file_handle))
    {
        remove_ptr_to_tracked((int8*)p_file_handle);
    }
#endif

#if CONTAINER_BOUND_TEST
    assert_true(
#endif
        CloseHandle(p_file_handle)
#if CONTAINER_BOUND_TEST
    )
#endif
        ;
};

inline void FileNative::set_file_pointer(FileHandle& p_file_handle, uimax p_pointer)
{
#if CONTAINER_BOUND_TEST
    assert_true(
#endif
        SetFilePointer(p_file_handle, (LONG)p_pointer, 0, FILE_BEGIN)
#if CONTAINER_BOUND_TEST
        != INVALID_SET_FILE_POINTER)
#endif
        ;
};

inline void FileNative::delete_file(const Slice<int8>& p_path)
{
#if CONTAINER_BOUND_TEST
    assert_true(
#endif
        DeleteFile(p_path.Begin)
#if CONTAINER_BOUND_TEST
    )
#endif
        ;
};

inline int8 FileNative::handle_is_valid(const FileHandle& p_file_handle)
{
    return p_file_handle != INVALID_HANDLE_VALUE;
};

#endif

struct File
{
    union
    {
        Span<int8> path_allocated;
        Slice<int8> path_slice;
    };
    FileHandle native_handle;

    inline static File create(const Slice<int8>& p_path)
    {
        File l_file = create_silent(p_path);
#if CONTAINER_BOUND_TEST
        assert_true(l_file.is_valid());
#endif
        return l_file;
    };

    inline static File open(const Slice<int8>& p_path)
    {
        File l_file = open_silent(p_path);
#if CONTAINER_BOUND_TEST
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

    inline void erase_with_slicepath()
    {
        this->free();
        FileNative::delete_file(this->path_slice);
    };

    inline void erase_with_spanpath()
    {
        this->free();
        FileNative::delete_file(this->path_allocated.slice);
        this->path_allocated.free();
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

    inline static File open_silent(const Slice<int8>& p_path)
    {
        File l_file;
        l_file.path_slice = p_path;
        l_file.native_handle = FileNative::open_file(p_path);
        return l_file;
    };
};
