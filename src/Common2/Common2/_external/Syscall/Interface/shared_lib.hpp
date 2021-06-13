#pragma once

struct shared_lib{
    int8* ptr;
};
struct shared_lib_proc_address{
    int8* ptr;
};

shared_lib shared_lib_load(const Slice<int8>& p_path);
shared_lib_proc_address shared_lib_get_procaddress(const shared_lib p_shader_lib, const Slice<int8>& p_proc_name);
