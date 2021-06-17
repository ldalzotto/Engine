#pragma once

enum class GPUExtension
{
    WINDOW_PRESENT = 0
};

#define GPU_DECLARE_TOKEN(p_name)                                                                                                                                                                      \
    struct _##p_name                                                                                                                                                                                   \
    {                                                                                                                                                                                                  \
    };                                                                                                                                                                                                 \
    using p_name = Token<_##p_name>

namespace gpu
{

using LayerConstString = SliceN<int8, 50>;

void layer_push_debug_layers(VectorSlice<LayerConstString>& p_layers);

GPU_DECLARE_TOKEN(Device);
GPU_DECLARE_TOKEN(Queue);
GPU_DECLARE_TOKEN(Instance);
GPU_DECLARE_TOKEN(Surface);
GPU_DECLARE_TOKEN(Debugger);

struct ApplicationInfo
{
    Slice<LayerConstString> enabled_layer_names;
    Slice<GPUExtension> enabled_extensions;
};

Instance create_instance(const ApplicationInfo& p_application_info);

Debugger initialize_debug_callback(Instance p_instance);
} // namespace gpu

#undef GPU_DECLARE_TOKEN