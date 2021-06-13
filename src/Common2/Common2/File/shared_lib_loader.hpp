#pragma once

struct SharedLibLoader
{
    inline static shared_lib load(const Slice<int8>& p_path)
    {
        if (!p_path.is_null_terminated())
        {
            String l_path = String::allocate_elements(p_path);
            shared_lib l_lib = shared_lib_load(l_path.to_slice());
            l_path.free();
            return l_lib;
        }
        else
        {
            return shared_lib_load(p_path);
        }
    };
    inline static shared_lib_proc_address get_procaddress(const shared_lib p_shader_lib, const Slice<int8>& p_proc_name)
    {
        if (!p_proc_name.is_null_terminated())
        {
            String l_proc_name = String::allocate_elements(p_proc_name);
            shared_lib_proc_address l_proc_address = shared_lib_get_procaddress(p_shader_lib, l_proc_name.to_slice());
            l_proc_name.free();
            return l_proc_address;
        }
        else
        {
            return shared_lib_get_procaddress(p_shader_lib, p_proc_name);
            // return (void*)GetProcAddress(p_shader_lib, p_proc_name.Begin);
        }
    };
};
