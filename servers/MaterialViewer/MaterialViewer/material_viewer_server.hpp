#include "Engine/engine.hpp"
#include "AssetCompiler/asset_compiler.hpp"

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

// In the future, We can add a variation of server and client taht doesn't consume json
struct MaterialViewerServerV2
{
    EngineRunnerThread engine_thread;
    MaterialViewerEngineUnit material_viewer_unit;

    inline static MaterialViewerServerV2 allocate()
    {
        MaterialViewerServerV2 l_return;
        l_return.engine_thread = EngineRunnerThread::allocate();
        return l_return;
    };

    inline void free()
    {
        // We don't know at compile time if the material_viewer_unit is freed or not.
        if (!this->material_viewer_unit.is_freed())
        {
            this->material_viewer_unit.free(this->engine_thread);
        };
        this->engine_thread.free();
    };

    inline void start_engine_thread()
    {
        this->engine_thread.start();
    };

    inline SocketRequestResponseConnection::ListenSendResponseReturnCode handle_request_json(const Slice<int8>& p_request, const Slice<int8>& p_response, Slice<int8>* in_out_response_slice)
    {
        SocketTypedHeaderRequestReader l_request = SocketTypedHeaderRequestReader::build(p_request);
        switch (*(MaterialViewerRequestCode*)l_request.code)
        {
        case MaterialViewerRequestCode::ENGINE_THREAD_START:
        {
            // this->engine_thread.start();
            this->material_viewer_unit = MaterialViewerEngineUnit::allocate();

            JSONDeserializer l_input_des = JSONDeserializer::start(l_request.body);
            MaterialViewerDomain::EngineThreadStartInputJSON l_input = MaterialViewerDomain::EngineThreadStartInputJSON::deserialize(l_input_des);

            this->material_viewer_unit.start(this->engine_thread, l_input.database, 400, 400);
            this->engine_thread.sync_wait_for_engine_execution_unit_to_be_allocated(this->material_viewer_unit.engine_execution_unit);

            l_input_des.free();

            *in_out_response_slice = SocketTypedHeaderRequestWriter::set((int32)MaterialViewerRequestCode::ENGINE_THREAD_START_RETURN, l_request.header, Slice<int8>::build_default(), p_response);

            return SocketRequestResponseConnection::ListenSendResponseReturnCode::SEND_RESPONSE;
        }
        break;
        case MaterialViewerRequestCode::ENGINE_THREAD_STOP:
        {
            this->material_viewer_unit.stop(this->engine_thread);

            *in_out_response_slice = SocketTypedHeaderRequestWriter::set((int32)MaterialViewerRequestCode::ENGINE_THREAD_STOP_RETURN, l_request.header, Slice<int8>::build_default(), p_response);

            return SocketRequestResponseConnection::ListenSendResponseReturnCode::SEND_RESPONSE;
        }
        break;
        case MaterialViewerRequestCode::SET_MATERIAL_AND_MESH:
        {
            JSONDeserializer l_input_des = JSONDeserializer::start(l_request.body);

            MaterialViewerDomain::SetMaterialAndMeshInputJSON l_input = MaterialViewerDomain::SetMaterialAndMeshInputJSON::deserialize(l_input_des);

            this->material_viewer_unit.set_new_material(HashSlice(l_input.material));
            this->material_viewer_unit.set_new_mesh(HashSlice(l_input.mesh));

            l_input_des.free();

            *in_out_response_slice = SocketTypedHeaderRequestWriter::set((int32)MaterialViewerRequestCode::SET_MATERIAL_AND_MESH_RETURN, l_request.header, Slice<int8>::build_default(), p_response);

            return SocketRequestResponseConnection::ListenSendResponseReturnCode::SEND_RESPONSE;
        }
        break;
        case MaterialViewerRequestCode::GET_ALL_MATERIALS:
        {
            DatabaseConnection& l_database_connection = this->engine_thread.engines.get(this->material_viewer_unit.engine_execution_unit).engine.database_connection;
            AssetMetadataDatabase l_asset_metadata_database = AssetMetadataDatabase::allocate(l_database_connection);
            AssetMetadataDatabase::Paths l_material_paths = l_asset_metadata_database.get_all_path_from_type(l_database_connection, AssetType_Const::MATERIAL_NAME);

            JSONSerializer l_json_serializer = JSONSerializer::allocate_default();
            l_json_serializer.start();
            MaterialViewerDomain::GetAllMaterialsOutputJSON::serialize_from_path(l_json_serializer, l_material_paths.data);
            l_json_serializer.end();

            l_material_paths.free();

            *in_out_response_slice = SocketTypedHeaderRequestWriter::set((int32)MaterialViewerRequestCode::GET_ALL_MATERIALS_RETURN, l_request.header, l_json_serializer.output.to_slice(), p_response);

            l_json_serializer.free();
            l_asset_metadata_database.free(l_database_connection);

            return SocketRequestResponseConnection::ListenSendResponseReturnCode::SEND_RESPONSE;
        }
        break;
        case MaterialViewerRequestCode::GET_ALL_MESH:
        {
            DatabaseConnection& l_database_connection = this->engine_thread.engines.get(this->material_viewer_unit.engine_execution_unit).engine.database_connection;
            AssetMetadataDatabase l_asset_metadata_database = AssetMetadataDatabase::allocate(l_database_connection);
            AssetMetadataDatabase::Paths l_mesh_paths = l_asset_metadata_database.get_all_path_from_type(l_database_connection, AssetType_Const::MESH_NAME);

            JSONSerializer l_json_input_serializer = JSONSerializer::allocate_default();
            l_json_input_serializer.start();
            MaterialViewerDomain::GetAllMeshesOutputJSON::serialize_from_path(l_json_input_serializer, l_mesh_paths.data);
            l_json_input_serializer.end();

            l_mesh_paths.free();

            *in_out_response_slice =
                SocketTypedHeaderRequestWriter::set((int32)MaterialViewerRequestCode::GET_ALL_MESH_RETURN, l_request.header, l_json_input_serializer.output.to_slice(), p_response);

            l_json_input_serializer.free();
            l_asset_metadata_database.free(l_database_connection);

            return SocketRequestResponseConnection::ListenSendResponseReturnCode::SEND_RESPONSE;
        }
        break;
        default:
            break;
        }

        return SocketRequestResponseConnection::ListenSendResponseReturnCode::NOTHING;
    };
};

struct MaterialViewerServerJSONThread
{
    struct Exec
    {
        MaterialViewerServerJSONThread* thiz;
        inline void operator()() const
        {
            thiz->thread.server.listen_request_response(*thiz->thread.input.ctx, [&](const Slice<int8>& p_request, const Slice<int8>& p_response, Slice<int8>* in_out_response_slice) {
                return thiz->material_viewer_server_v2.handle_request_json(p_request, p_response, in_out_response_slice);
            });
        };
    } exec;

    SocketSocketServerSingleClientThread<Exec> thread;
    MaterialViewerServerV2 material_viewer_server_v2;

    inline void start(SocketContext& p_ctx, const int32 p_port)
    {
        this->exec = Exec{this};
        this->material_viewer_server_v2 = MaterialViewerServerV2::allocate();
        this->material_viewer_server_v2.start_engine_thread();
        this->thread.start(&p_ctx, p_port, &this->exec);
    };

    inline void free(SocketContext& p_ctx)
    {
        this->thread.free(p_ctx);
        this->material_viewer_server_v2.free();
    };
};
