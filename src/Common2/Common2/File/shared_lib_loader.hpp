#pragma once

#if 0
typedef HMODULE
#ifdef _WIN32
    sharedlib_t;
#endif

struct SharedLibLoader
{
    static sharedlib_t load(const char* p_path);
    static void* get_procaddress(const sharedlib_t p_shader_lib, const char* p_proc_name);
};

#ifdef _WIN32

inline sharedlib_t SharedLibLoader::load(const char* p_path)
{
    /*
    LPCWSTR l_path  = (LPCWSTR)p_path;
    const size_t l_length = strlen(p_path) +1;
    wchar_t* w_path = (wchar_t*)malloc(sizeof(wchar_t ) * l_length);
    size_t out_size;
    mbstowcs_s(&out_size, w_path, l_length, p_path, l_length);
    LoadLibraryA(p_path);
     */
    sharedlib_t l_lib = LoadLibrary(p_path);
    // free(w_path);
    return l_lib;
};

inline void* SharedLibLoader::get_procaddress(const sharedlib_t p_shader_lib, const char* p_proc_name)
{
    return (void*)GetProcAddress(p_shader_lib, p_proc_name);
};

#endif

#endif