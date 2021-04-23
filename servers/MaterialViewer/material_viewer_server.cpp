
#define __SOCKET_ENABLED 1
#include "Engine/engine.hpp"
#include "AssetCompiler/asset_compiler.hpp"

// #include <iphlpapi.h>
// #include <bcrypt.h>

enum class MaterialViewerRequestCode : int32
{
    ENGINE_THREAD_START = 0,
    ENGINE_THREAD_STOP = 1,
    SET_MATERIAL_AND_MESH = 2,
    GET_ALL_MATERIALS = 4,
    GET_ALL_MATERIALS_RETURN = 5,
    GET_ALL_MESH = 6,
    GET_ALL_MESH_RETURN = 7,
};

struct MaterialViewerEngineUnit
{
    int8 is_running;
    Token<EngineExecutionUnit> engine_execution_unit;

    Token<Node> camera_node;
    Token<Node> material_node;
    Token<MeshRendererComponent> material_node_meshrenderer;

    struct SharedRessources
    {
        uimax material_hash;
        uimax mesh_hash;
        int8 change_requested;
    };

    MutexNative<SharedRessources> shared;

    inline static MaterialViewerEngineUnit allocate()
    {
        MaterialViewerEngineUnit l_return{};
        l_return.set_sefault_values();
        l_return.shared = MutexNative<SharedRessources>::allocate();
        return l_return;
    };

    inline void set_sefault_values()
    {
        this->engine_execution_unit = token_build_default<EngineExecutionUnit>();
        this->material_node_meshrenderer = token_build_default<MeshRendererComponent>();
        this->camera_node = token_build_default<Node>();
        this->material_node = token_build_default<Node>();
    };

    inline void free(EngineRunnerThread& p_engine_runner)
    {
        this->stop(p_engine_runner);
        this->shared.free();
    };

    inline int8 is_freed()
    {
        return token_equals(this->engine_execution_unit, token_build_default<EngineExecutionUnit>());
    };

    inline void start(EngineRunnerThread& p_engine_runner, const Slice<int8> p_asset_database, const uint32 p_width, const uint32 p_height)
    {
        this->is_running = 1;
        this->engine_execution_unit = p_engine_runner.allocate_engine_execution_unit(
            p_asset_database, p_width, p_height, EngineExternalStepCallback{(void*)this, (EngineExternalStepCallback::cb_t)MaterialViewerEngineUnit::step},
            EngineExecutionUnit::CleanupCallback{(void*)this, (EngineExecutionUnit::CleanupCallback::cb_t)MaterialViewerEngineUnit::cleanup_ressources});
    };

    inline void stop(EngineRunnerThread& p_engine_runner)
    {
        if (this->is_running)
        {
            p_engine_runner.free_engine_execution_unit_sync(this->engine_execution_unit);
            this->set_sefault_values();
        }
    };

    inline void restart(EngineRunnerThread& p_engine_runner, const Slice<int8> p_asset_database, const uint32 p_width, const uint32 p_height)
    {
        this->stop(p_engine_runner);
        this->start(p_engine_runner, p_asset_database, p_width, p_height);
    };

    inline void set_new_material(const hash_t p_new_material)
    {
        this->shared.acquire([&](SharedRessources& p_shared) {
            p_shared.change_requested = 1;
            p_shared.material_hash = p_new_material;
        });
    };

    inline void set_new_mesh(const hash_t p_new_mesh)
    {
        this->shared.acquire([&](SharedRessources& p_shared) {
            p_shared.change_requested = 1;
            p_shared.mesh_hash = p_new_mesh;
        });
    };

  private:
    inline static void step(const EngineExternalStep p_step, Engine& p_engine, MaterialViewerEngineUnit* thiz)
    {
        if (p_step == EngineExternalStep::BEFORE_UPDATE)
        {
            float32 l_deltatime = DeltaTime(p_engine);
            uimax l_frame_count = FrameCount(p_engine);
            if (l_frame_count == 1)
            {
                thiz->camera_node = CreateNode(p_engine, transform_const::ORIGIN);
                thiz->material_node = CreateNode(p_engine, transform{v3f{0.0f, 0.0f, 5.0f}, quat_const::IDENTITY, v3f_const::ONE.vec3});
                NodeAddCamera(p_engine, thiz->camera_node, CameraComponent::Asset{1.0f, 30.0f, 45.0f});
            }
            thiz->shared.acquire([&](SharedRessources& p_shared) {
                if (p_shared.change_requested)
                {
                    if (token_value(thiz->material_node_meshrenderer) != -1)
                    {
                        NodeRemoveMeshRenderer(p_engine, thiz->material_node);
                    }
                    thiz->material_node_meshrenderer = NodeAddMeshRenderer(p_engine, thiz->material_node, p_shared.material_hash, p_shared.mesh_hash);
                    p_shared.change_requested = 0;
                }
            });

            NodeAddWorldRotation(p_engine, thiz->material_node, quat::rotate_around(v3f_const::UP, 3 * l_deltatime));
        }
    };

    inline static void cleanup_ressources(Engine& p_engine, MaterialViewerEngineUnit* thiz)
    {
        RemoveNode(p_engine, thiz->camera_node);
        RemoveNode(p_engine, thiz->material_node);
        thiz->material_node_meshrenderer = token_build_default<MeshRendererComponent>();
        thiz->is_running = 0;
    };
};

struct MaterialViewerServer
{
    SocketServer socket_server;
    EngineRunnerThread engine_thread;
    MaterialViewerEngineUnit material_viewer_unit;

    inline static MaterialViewerServer allocate(SocketContext& p_ctx, const int32 p_port)
    {
        MaterialViewerServer l_return;
        l_return.socket_server = SocketServer::allocate(p_ctx, p_port);
        l_return.engine_thread = EngineRunnerThread::allocate();
        return l_return;
    };

    inline void start_engine_thread()
    {
        this->engine_thread.start();
    };

    inline void wait_for_client(SocketContext& p_ctx)
    {
        this->socket_server.wait_for_client(p_ctx);
    };

    inline void listen_for_requests(SocketContext& p_ctx)
    {
        SocketCommunication::listen_for_requests(p_ctx, this->socket_server.client_socket, this->socket_server.client_socket, [&](const Slice<int8>& p_request, Slice<int8>& p_response) {
            SocketRequest l_request = SocketRequest::build(p_request);
            MaterialViewerRequestCode l_request_code = (MaterialViewerRequestCode)l_request.code;
            this->handle_request(l_request_code, l_request.payload, p_response);
            return SocketCommunication::RequestListenerReturnCode::SEND_RESPONSE;
        });

        if (!this->material_viewer_unit.is_freed())
        {
            this->material_viewer_unit.free(this->engine_thread);
            // this->engine_thread.free();
        };
    };

  private:
    inline void handle_request(const MaterialViewerRequestCode p_code, const Slice<int8>& p_payload, const Slice<int8>& p_response)
    {
        switch (p_code)
        {
        case MaterialViewerRequestCode::ENGINE_THREAD_START:
        {
            // this->engine_thread.start();
            this->material_viewer_unit = MaterialViewerEngineUnit::allocate();

            BinaryDeserializer l_payload_deserializer = BinaryDeserializer::build(p_payload);
            Slice<int8> l_input = l_payload_deserializer.slice();
            JSONDeserializer l_input_des = JSONDeserializer::start(l_input);
            l_input_des.next_field("database");
            Slice<int8> l_database_path = l_input_des.get_currentfield().value;
            this->material_viewer_unit.start(this->engine_thread, l_database_path, 400, 400);
            this->engine_thread.sync_wait_for_engine_execution_unit_to_be_allocated(this->material_viewer_unit.engine_execution_unit);

            l_input_des.free();
        }
        break;
        case MaterialViewerRequestCode::ENGINE_THREAD_STOP:
        {
            this->material_viewer_unit.stop(this->engine_thread);
            // this->engine_thread.free();

            Slice<int8> l_response = p_response;
        }
        break;
        case MaterialViewerRequestCode::SET_MATERIAL_AND_MESH:
        {
            BinaryDeserializer l_payload_deserializer = BinaryDeserializer::build(p_payload);
            Slice<int8> l_input = l_payload_deserializer.slice();
            JSONDeserializer l_input_des = JSONDeserializer::start(l_input);

            l_input_des.next_field("material");
            Slice<int8> l_material = l_input_des.get_currentfield().value;
            l_input_des.next_field("mesh");
            Slice<int8> l_mesh = l_input_des.get_currentfield().value;

            this->material_viewer_unit.set_new_material(HashSlice(l_material));
            this->material_viewer_unit.set_new_mesh(HashSlice(l_mesh));

            l_input_des.free();
        }
        break;
        case MaterialViewerRequestCode::GET_ALL_MATERIALS:
        {
            DatabaseConnection& l_database_connection = this->engine_thread.engines.get(this->material_viewer_unit.engine_execution_unit).engine.database_connection;
            AssetMetadataDatabase l_asset_metadata_database = AssetMetadataDatabase::allocate(l_database_connection);
            AssetMetadataDatabase::Paths l_material_paths = l_asset_metadata_database.get_all_path_from_type(l_database_connection, AssetType_Const::MATERIAL_NAME);

            JSONSerializer l_json_deser = JSONSerializer::allocate_default();
            l_json_deser.start();
            l_json_deser.start_array(slice_int8_build_rawstr("materials"));
            for (loop(i, 0, l_material_paths.data.Size))
            {
                l_json_deser.push_array_field(l_material_paths.data.get(i).slice);
            }
            l_json_deser.end_array();
            l_json_deser.end();

            l_material_paths.free();

            Slice<int8> l_response = p_response;
            BinarySerializer::type<int32>(&l_response, (int32)MaterialViewerRequestCode::GET_ALL_MATERIALS_RETURN);
            BinarySerializer::slice(&l_response, l_json_deser.output.to_slice());

            l_json_deser.free();
            l_asset_metadata_database.free(l_database_connection);
        }
        break;
        case MaterialViewerRequestCode::GET_ALL_MESH:
        {
            DatabaseConnection& l_database_connection = this->engine_thread.engines.get(this->material_viewer_unit.engine_execution_unit).engine.database_connection;
            AssetMetadataDatabase l_asset_metadata_database = AssetMetadataDatabase::allocate(l_database_connection);
            AssetMetadataDatabase::Paths l_mesh_paths = l_asset_metadata_database.get_all_path_from_type(l_database_connection, AssetType_Const::MESH_NAME);

            JSONSerializer l_json_deser = JSONSerializer::allocate_default();
            l_json_deser.start();
            l_json_deser.start_array(slice_int8_build_rawstr("meshes"));
            for (loop(i, 0, l_mesh_paths.data.Size))
            {
                l_json_deser.push_array_field(l_mesh_paths.data.get(i).slice);
            }
            l_json_deser.end_array();
            l_json_deser.end();

            l_mesh_paths.free();

            Slice<int8> l_response = p_response;
            BinarySerializer::type<int32>(&l_response, (int32)MaterialViewerRequestCode::GET_ALL_MESH_RETURN);
            BinarySerializer::slice(&l_response, l_json_deser.output.to_slice());

            l_json_deser.free();
            l_asset_metadata_database.free(l_database_connection);
        }
        break;
        default:
            break;
        }
    };
};

int main()
{
    SocketContext l_socket_context = SocketContext::allocate();
    MaterialViewerServer l_server = MaterialViewerServer::allocate(l_socket_context, 8000);
    l_server.start_engine_thread();

    while (true)
    {
        l_server.wait_for_client(l_socket_context);
        l_server.listen_for_requests(l_socket_context);
    }

    l_socket_context.free();
    memleak_ckeck();

    return 0;
};