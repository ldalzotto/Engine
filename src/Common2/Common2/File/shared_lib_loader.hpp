#pragma once

typedef HMODULE
#ifdef _WIN32
    sharedlib_t;
#else
    int8*;
#endif

struct SharedLibLoader
{
    static sharedlib_t load(const Slice<int8>& p_path);
    static void* get_procaddress(const sharedlib_t p_shader_lib, const Slice<int8>& p_proc_name);
};

#ifdef _WIN32

inline sharedlib_t SharedLibLoader::load(const Slice<int8>& p_path)
{
    if (!p_path.is_null_terminated())
    {
        String l_path = String::allocate_elements(p_path);
        sharedlib_t l_lib = LoadLibrary(l_path.Memory.Memory.Memory);
        l_path.free();
        return l_lib;
    }
    else
    {
        return LoadLibrary(p_path.Begin);
    }
};

inline void* SharedLibLoader::get_procaddress(const sharedlib_t p_shader_lib, const Slice<int8>& p_proc_name)
{
    if (!p_proc_name.is_null_terminated())
    {
        String l_proc_name = String::allocate_elements(p_proc_name);
        void* l_proc_address = (void*)GetProcAddress(p_shader_lib, l_proc_name.get_memory());
        l_proc_name.free();
        return l_proc_address;
    }
    else
    {
        return (void*)GetProcAddress(p_shader_lib, p_proc_name.Begin);
    }
};

#endif
