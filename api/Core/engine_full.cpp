
#include "Engine/engine.hpp"

#define EXPORT __declspec(dllexport)

using Engine_t = Engine_Scene_GPU_AssetDatabase_D3Renderer_Window_Present;

enum class EngineFunctions : int8
{
    SpawnEngine = 0,
    DestroyEngine,
    MainLoopNonBlocking,
    FrameBefore,
    FrameAfter,
    DeltaTime,
    FrameCount,
    Node_CreateRoot,
    Node_Remove,
    Node_GetLocalPosition,
    Node_AddWorldRotation,
    Node_AddCamera,
    Node_AddMeshRenderer
};

extern "C"
{
    EXPORT inline void EntryPoint(const Slice<int8>& p_input, VectorSlice<int8>& out)
    {
        BinaryDeserializer l_deserializer = BinaryDeserializer::build(p_input);
        iVector_v2<VectorSlice<int8>> l_out = iVector_v2<VectorSlice<int8>>{out};
        EngineFunctions l_function = *l_deserializer.type<EngineFunctions>();

        switch (l_function)
        {
        case EngineFunctions::SpawnEngine:
        {
            Engine_t::RuntimeConfiguration l_configuration;
            l_configuration.core.target_framerate_mics = (time_t)(1000000 / 60);
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
        case EngineFunctions::MainLoopNonBlocking:
        {
            Engine_t* l_engine = *l_deserializer.type<Engine_t*>();
            EngineLoopState l_loop_state = iEngine<Engine_t>{*l_engine}.main_loop_non_blocking();
            BinarySerializer::type(l_out, l_loop_state);
        }
        break;
        case EngineFunctions::FrameBefore:
        {
            Engine_t* l_engine = *l_deserializer.type<Engine_t*>();
            iEngine<Engine_t>{*l_engine}.frame_before();
        }
        break;
        case EngineFunctions::FrameAfter:
        {
            Engine_t* l_engine = *l_deserializer.type<Engine_t*>();
            iEngine<Engine_t>{*l_engine}.frame_after();
        }
        break;
        case EngineFunctions::DeltaTime:
        {
            Engine_t* l_engine = *l_deserializer.type<Engine_t*>();
            BinarySerializer::type(l_out, iEngine<Engine_t>{*l_engine}.deltatime());
        }
        break;
        case EngineFunctions::FrameCount:
        {
            Engine_t* l_engine = *l_deserializer.type<Engine_t*>();
            BinarySerializer::type(l_out, iEngine<Engine_t>{*l_engine}.frame_count());
        }
        break;
        case EngineFunctions::Node_CreateRoot:
        {
            Engine_t* l_engine = *l_deserializer.type<Engine_t*>();
            transform l_position = MathBinaryDeserialization::_transform(l_deserializer);
            Token<Node> l_node = iEngine<Engine_t>{*l_engine}.create_node(l_position);
            BinarySerializer::type(l_out, l_node);
        }
        break;
        case EngineFunctions::Node_Remove:
        {
            Engine_t* l_engine = *l_deserializer.type<Engine_t*>();
            Token<Node> l_node = *l_deserializer.type<Token<Node>>();
            iEngine<Engine_t>{*l_engine}.remove_node(l_node);
        }
        break;
        case EngineFunctions::Node_GetLocalPosition:
        {
            Engine_t* l_engine = *l_deserializer.type<Engine_t*>();
            Token<Node> l_node = *l_deserializer.type<Token<Node>>();
            BinarySerializer::type(l_out, iEngine<Engine_t>{*l_engine}.node_get_localposition(l_node));
        }
        break;
        case EngineFunctions::Node_AddWorldRotation:
        {
            Engine_t* l_engine = *l_deserializer.type<Engine_t*>();
            Token<Node> l_node = *l_deserializer.type<Token<Node>>();
            quat* l_world_rotation_delta = l_deserializer.type<quat>();
            iEngine<Engine_t>{*l_engine}.node_add_worldrotation(l_node, *l_world_rotation_delta);
        }
        break;
        case EngineFunctions::Node_AddCamera:
        {
            Engine_t* l_engine = *l_deserializer.type<Engine_t*>();
            Token<Node> l_node = *l_deserializer.type<Token<Node>>();
            CameraComponent::Asset* l_cameracomponent_asset = l_deserializer.type<CameraComponent::Asset>();
            CameraComponent& l_camera_component = iEngine<Engine_t>{*l_engine}.node_add_camera(l_node, *l_cameracomponent_asset);
            BinarySerializer::type(l_out, &l_camera_component);
        }
        break;
        case EngineFunctions::Node_AddMeshRenderer:
        {
            Engine_t* l_engine = *l_deserializer.type<Engine_t*>();
            Token<Node> l_node = *l_deserializer.type<Token<Node>>();
            Slice<int8> l_material_path = l_deserializer.slice();
            Slice<int8> l_mesh_path = l_deserializer.slice();
            Token<MeshRendererComponent> l_mesh_renderer = iEngine<Engine_t>{*l_engine}.node_add_meshrenderer(l_node, HashFunctions::hash(l_material_path), HashFunctions::hash(l_mesh_path));
            BinarySerializer::type(l_out, l_mesh_renderer);
        }
        break;
        }
    };

    EXPORT inline void Finalize()
    {
        memleak_ckeck();
    };
}
