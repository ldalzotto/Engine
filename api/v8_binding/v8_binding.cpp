// #include "./v8/v8.h"

#if node_gyp

#include <node.h>

#else

#include "v8/v8.h"
#ifndef NODE_SET_METHOD
#define NODE_SET_METHOD(p_exports, p_name, p_entry)
#endif
#ifndef NODE_GYP_MODULE_NAME
#define NODE_GYP_MODULE_NAME "test"
#endif
#ifndef NODE_MODULE
#define NODE_MODULE(p_module_name, p_initialize_function)
#endif

#endif

#define __SQLITE_ENABLED 0
#define __NATIVE_WINDOW_ENABLED 0
#include "Common2/common2.hpp"

thread_t g_initialization_thread;
Vector<int8> g_arg_buffer;
Vector<int8> g_out_buffer;

inline void clear_global_buffers()
{
    g_arg_buffer.clear();
    g_out_buffer.clear();
};

inline void assert_same_thread()
{
#if __DEBUG
    assert_true(g_initialization_thread == Thread::get_current_thread());
#endif
};

using EntryPointFunc = void (*)(const Slice<int8>& p_input, VectorSlice<int8>& out);
EntryPointFunc pp_EntryPoint;

void LoadDynamicLib(const v8::FunctionCallbackInfo<v8::Value>& args)
{
    assert_same_thread();
    v8::Isolate* isolate = args.GetIsolate();
    // v8::Local<v8::Value> l_path_value = args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked();
    v8::String::Utf8Value l_shared_library_path(isolate, args[0]);
    char* l_path = l_shared_library_path.operator*();
    sharedlib_t l_engine_dll = SharedLibLoader::load(slice_int8_build_rawstr(l_path));
    assert_true(l_engine_dll != 0);
    pp_EntryPoint = (EntryPointFunc)SharedLibLoader::get_procaddress(l_engine_dll, slice_int8_build_rawstr("EntryPoint"));
    assert_true(pp_EntryPoint != 0);
};

void node_push_args(const v8::FunctionCallbackInfo<v8::Value>& args)
{
    v8::Isolate* isolate = args.GetIsolate();

    for (int i = 0; i < args.Length(); i++)
    {
        if (args[i]->IsArrayBuffer())
        {
            v8::ArrayBuffer::Contents l_contents = args[i]->ToObject(isolate->GetCurrentContext()).ToLocalChecked().As<v8::ArrayBuffer>()->GetContents();
            g_arg_buffer.push_back_array(Slice<int8>::build_memory_elementnb((int8*)l_contents.Data(), l_contents.ByteLength()));
        }
        else if (args[i]->IsFunction())
        {
            v8::Local<v8::Function> l_function = args[i]->ToObject(isolate->GetCurrentContext()).ToLocalChecked().As<v8::Function>();
            g_arg_buffer.push_back_array(Slice<int8>::build_memory_elementnb((int8*)&l_function, sizeof(v8::Local<v8::Function>)));
        }
    }
};

void EntryPoint(const v8::FunctionCallbackInfo<v8::Value>& args)
{
    assert_same_thread();
    clear_global_buffers();
    node_push_args(args);
    VectorSlice<int8> l_return = VectorSlice<int8>::build(g_out_buffer.Memory.slice, 0);
    pp_EntryPoint(g_arg_buffer.to_slice(), l_return);
    args.GetReturnValue().Set(v8::ArrayBuffer::New(args.GetIsolate(), g_out_buffer.Memory.Memory, l_return.get_size()));
};

void Finalize(const v8::FunctionCallbackInfo<v8::Value>& args)
{
    assert_same_thread();
    g_arg_buffer.free();
    g_out_buffer.free();

    memleak_ckeck();
};

void Initialize(v8::Local<v8::Object> exports)
{
    g_initialization_thread = Thread::get_current_thread();
    g_arg_buffer = Vector<int8>::allocate(0);
    g_out_buffer = Vector<int8>::allocate(1000000);

    NODE_SET_METHOD(exports, "Finalize", Finalize);
    NODE_SET_METHOD(exports, "LoadDynamicLib", LoadDynamicLib);
    NODE_SET_METHOD(exports, "EntryPoint", EntryPoint);
}

NODE_MODULE(NODE_GYP_MODULE_NAME, Initialize)