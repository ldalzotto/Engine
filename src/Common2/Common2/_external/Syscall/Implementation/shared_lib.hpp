#pragma once

shared_lib shared_lib_load(const Slice<int8>& p_path)
{
    shared_lib l_shared_lib;
    l_shared_lib.ptr = (int8*)LoadLibrary(p_path.Begin);
    return l_shared_lib;
};

shared_lib_proc_address shared_lib_get_procaddress(const shared_lib p_shader_lib, const Slice<int8>& p_proc_name)
{
    shared_lib_proc_address l_proc_address;
    l_proc_address.ptr = (int8*)GetProcAddress((HMODULE)p_shader_lib.ptr, p_proc_name.Begin);
    return l_proc_address;
};