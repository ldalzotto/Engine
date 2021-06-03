
#include "Engine/engine.hpp"

#define EXPORT __declspec(dllexport)

using Engine_t = Engine_Scene_GPU_AssetDatabase_D3Renderer_Window_Present;

enum class EngineFunctions : int8
{
    SpawnEngine = 0,
    DestroyEngine = 1
};

extern "C"
{
    EXPORT inline void EntryPoint(const Slice<int8>& p_input, VectorSlice<int8>& out)
    {
        BinaryDeserializer l_deserializer = BinaryDeserializer::build(p_input);
        iVector<VectorSlice<int8>> l_out = iVector<VectorSlice<int8>>{out};
        EngineFunctions l_function = *l_deserializer.type<EngineFunctions>();

        switch (l_function)
        {
        case EngineFunctions::SpawnEngine:
        {
            Engine_t::RuntimeConfiguration l_configuration;
            l_configuration.core.target_framerate_mics = 0;
            l_configuration.database_path = l_deserializer.slice();
            l_configuration.render_size = v2ui{400, 400};
            l_configuration.render_target_host_readable = 0;

            Engine_t* l_engine = (Engine_t*)heap_malloc(sizeof(Engine_t));
            *l_engine = Engine_t::allocate(l_configuration);

            BinarySerializer::type(l_out, l_engine);
        }
        break;
        case EngineFunctions::DestroyEngine:
        {
            Engine_t* l_engine = *l_deserializer.type<Engine_t*>();
            l_engine->free();
            heap_free((int8*)l_engine);
        }
        break;
        }
    };

    EXPORT inline void Finalize()
    {
        memleak_ckeck();
    };
}
