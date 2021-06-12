#pragma once

struct FileHandle
{
#if __MEMLEAK
    int8* memleak_ptr;
#endif
    file_native handle;
};

struct FileNative
{
    inline static FileHandle create_file(const Slice<int8>& p_path)
    {
        FileHandle l_handle;

        // We must do this check because the input path may not be null terminated and the win32 api CreateFile only allows null terminated string.
        // So if that's the case, we allocate a temporary string to be null terminated.
        if (p_path.get(p_path.Size - 1) != '\0' && p_path.Begin[p_path.Size] != '\0')
        {
            String l_file_path_null_terminated = String::allocate_elements(p_path);
            l_handle.handle = file_native_create_file_unchecked(l_file_path_null_terminated.to_slice());
            l_file_path_null_terminated.free();
        }
        else
        {
            l_handle.handle = file_native_create_file_unchecked(p_path);
        }

#if __MEMLEAK
        if (file_native_handle_is_valid(l_handle.handle))
        {
            l_handle.memleak_ptr = heap_malloc(1);
        }
#endif
        return l_handle;
    };

    inline static FileHandle open_file(const Slice<int8>& p_path)
    {
        FileHandle l_handle;
        // We must do this check because the input path may not be null terminated and the win32 api CreateFile only allows null terminated string.
        // So if that's the case, we allocate a temporary string to be null terminated.
        if (p_path.get(p_path.Size - 1) != '\0' && p_path.Begin[p_path.Size] != '\0')
        {
            String l_file_path_null_terminated = String::allocate_elements(p_path);
            l_handle.handle = file_native_open_file_unchecked(l_file_path_null_terminated.to_slice());
            l_file_path_null_terminated.free();
        }
        else
        {
            l_handle.handle = file_native_open_file_unchecked(p_path);
        }

#if __MEMLEAK
        if (file_native_handle_is_valid(l_handle.handle))
        {
            l_handle.memleak_ptr = heap_malloc(1);
        }
#endif
        return l_handle;
    }

    inline static void close_file(FileHandle& p_file_handle)
    {
#if __MEMLEAK
        if (file_native_handle_is_valid(p_file_handle.handle))
        {
            heap_free(p_file_handle.memleak_ptr);
        }
#endif

        file_native_close_file_unchecked(p_file_handle.handle);
        p_file_handle = FileHandle{0};
    };

    inline static void delete_file(const Slice<int8>& p_path)
    {
        // We must do this check because the input path may not be null terminated and the win32 api CreateFile only allows null terminated string.
        // So if that's the case, we allocate a temporary string to be null terminated.
        if (p_path.get(p_path.Size - 1) != '\0' && p_path.Begin[p_path.Size] != '\0')
        {
            String l_file_path_null_terminated = String::allocate_elements(p_path);
            file_native_delete_file_unchecked(l_file_path_null_terminated.to_slice());
            l_file_path_null_terminated.free();
        }
        else
        {
            file_native_delete_file_unchecked(p_path);
        }
    };
};

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
        return file_native_get_file_size(this->native_handle.handle);
    };

    inline uimax get_modification_ts()
    {
        return file_native_get_modification_ts(this->native_handle.handle);
    };

    inline void read_file(Slice<int8>* in_out_buffer) const
    {
        file_native_set_file_pointer(this->native_handle.handle, 0);
        uimax l_file_size = file_native_get_file_size(this->native_handle.handle);
#if __DEBUG
        assert_true(l_file_size <= in_out_buffer->Size);
#endif
        in_out_buffer->Size = l_file_size;
        file_native_read_buffer(this->native_handle.handle, in_out_buffer);
    };

    inline Span<int8> read_file_allocate() const
    {
        uimax l_file_size = file_native_get_file_size(this->native_handle.handle);
        Span<int8> l_buffer = Span<int8>::allocate(l_file_size);
        this->read_file(&l_buffer.slice);
        return l_buffer;
    };

    inline void write_file(const Slice<int8>& p_buffer)
    {
        file_native_set_file_pointer(this->native_handle.handle, 0);
        file_native_write_buffer(this->native_handle.handle, 0, p_buffer);
    };

    inline int8 is_valid()
    {
        return file_native_handle_is_valid(this->native_handle.handle);
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
