#include "Engine/engine.hpp"

#define ENGINE_API __declspec(dllexport)

enum class EngineAPIKey : uint32
{
    Undefined = 0,
    SpawnEngine = 1,
    DestroyEngine = 2,
    SpawnEngine_v2 = 3,
    MainLoop = 4,
    CreateNode = 5,
    FrameCount = 6,
    NodeAddCamera = 7,
    NodeAddMeshRenderer = 8,
    RemoveNode = 9,
    NodeAddWorldRotation = 10,
    DeltaTime = 11
};

// TODO -> add Initialize and Finalize ?
extern "C"
{
    ENGINE_API void __cdecl EntryPoint(void** argv, int argc)
    {
        EngineAPIKey l_key = *(EngineAPIKey*)argv[0];

        switch (l_key)
        {
        case EngineAPIKey::SpawnEngine:
        {
            Engine* l_engine = (Engine*)heap_malloc(sizeof(Engine));
            *l_engine = Engine::allocate(
                EngineConfiguration{slice_int8_build_rawstr_with_null_termination("E:/GameProjects/GameEngineLinux/_asset/asset/sandbox/d3renderer_cube/asset.db"), 0, v2ui{800, 600}, 1});
            *(Engine**)argv[1] = l_engine;
        }
        break;
        case EngineAPIKey::SpawnEngine_v2:
        {

            Engine* l_engine = (Engine*)heap_malloc(sizeof(Engine));

            Slice<int8> l_input = Slice<int8>::build_memory_elementnb((int8*)argv[2], -1);

            BinaryDeserializer l_input_deserilizer = BinaryDeserializer::build(l_input);
            Slice<int8> l_asset_database_path =  l_input_deserilizer.slice();

            uint32 l_width = *l_input_deserilizer.type<uint32>();
            uint32 l_height = *l_input_deserilizer.type<uint32>();

            *l_engine = Engine::allocate(EngineConfiguration{l_asset_database_path, 0, v2ui{l_width, l_height}, 1});
            *(Engine**)argv[1] = l_engine;
        }
        break;
        case EngineAPIKey::MainLoop:
        {
            Engine** l_engine = (Engine**)argv[1];

            Vector<int8> l_external_step_vp_input = Vector<int8>::allocate(0);

            typedef void (*l_external_step_vp)(void** p_args, int p_argc);
            struct engine_cb
            {
                Vector<int8>* l_external_step_vp_input;
                void* js_cb;
                l_external_step_vp cpp_cb;
                inline void step(EngineExternalStep p_step, Engine& p_engine) const
                {
                    BinarySerializer::type(l_external_step_vp_input, (uint32)p_step);
                    BinarySerializer::type(l_external_step_vp_input, &p_engine);

                    Slice<int8> l_args_binary = l_external_step_vp_input->to_slice();

                    void* l_args[3] = {this->js_cb, &l_args_binary.Size, l_args_binary.Begin};

                    this->cpp_cb(l_args, 3);

                    l_external_step_vp_input->clear();
                };
            };

            l_external_step_vp cpp_callback = (l_external_step_vp)argv[3];
            (*l_engine)->main_loop(engine_cb{&l_external_step_vp_input, argv[2], cpp_callback});

            l_external_step_vp_input.free();
        }
        break;
        case EngineAPIKey::CreateNode:
        {
            Engine** l_engine = (Engine**)argv[1];

            BinaryDeserializer l_binary_deserializer = BinaryDeserializer::build(Slice<int8>::build_begin_end((int8*)argv[2], 0, -1));
            transform l_transform = MathBinaryDeserialization::_transform(l_binary_deserializer);

            Token(Node) l_node = CreateNode(**l_engine, l_transform);

            *(Token(Node)*)argv[3] = l_node;
        }
        break;
        case EngineAPIKey::RemoveNode:
        {
            Engine** l_engine = (Engine**)argv[1];
            Token(Node)* l_node = (Token(Node)*)argv[2];

            RemoveNode(**l_engine, *l_node);
        }
        break;
        case EngineAPIKey::NodeAddWorldRotation:
        {
            Engine** l_engine = (Engine**)argv[1];
            Token(Node)* l_node = (Token(Node)*)argv[2];
            BinaryDeserializer l_binary_deserializer = BinaryDeserializer::build(Slice<int8>::build_begin_end((int8*)argv[3], 0, -1));
            quat l_delta = *l_binary_deserializer.type<quat>();
            NodeAddWorldRotation(**l_engine, *l_node, l_delta);
        }
        break;
        case EngineAPIKey::NodeAddCamera:
        {
            Engine** l_engine = (Engine**)argv[1];
            Token(Node)* l_node = (Token(Node)*)argv[2];
            NodeAddCamera(**l_engine, *l_node, CameraComponent::Asset::build_from_binary(Slice<int8>::build_begin_end((int8*)argv[3], 0, -1)));
        }
        break;
        case EngineAPIKey::NodeAddMeshRenderer:
        {
            Engine** l_engine = (Engine**)argv[1];
            Token(Node)* l_node = (Token(Node)*)argv[2];

            Slice<int8> l_input = Slice<int8>::build_memory_elementnb((int8*)argv[3], -1);
            BinaryDeserializer l_input_deserilizer = BinaryDeserializer::build(l_input);

            Slice<int8> l_asset_material_path = l_input_deserilizer.slice();
            Slice<int8> l_asset_mesh_path = l_input_deserilizer.slice();

            *(Token(MeshRendererComponent)*)argv[4] = NodeAddMeshRenderer(**l_engine, *l_node, HashRaw(l_asset_material_path.Begin), HashRaw(l_asset_mesh_path.Begin));
        }
        break;
        case EngineAPIKey::FrameCount:
        {
            Engine** l_engine = (Engine**)argv[1];
            *(size_t*)argv[2] = FrameCount(**l_engine);
        }
        break;
        case EngineAPIKey::DeltaTime:
        {
            Engine** l_engine = (Engine**)argv[1];
            *(float32*)argv[2] = DeltaTime(**l_engine);
        }
        break;
        case EngineAPIKey::DestroyEngine:
        {
            Engine** l_engine = (Engine**)argv[1];
            (*l_engine)->free();
            heap_free((int8*)*l_engine);
        }
        break;
        default:
            abort();
        }
    };
}
