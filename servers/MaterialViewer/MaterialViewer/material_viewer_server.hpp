
#define __SOCKET_ENABLED 1
#include "Engine/engine.hpp"
#include "AssetCompiler/asset_compiler.hpp"

// #include <iphlpapi.h>
// #include <bcrypt.h>

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

struct MaterialViewerServer
{
    SocketServerSingleClient socket_server;
    EngineRunnerThread engine_thread;
    MaterialViewerEngineUnit material_viewer_unit;

    inline static MaterialViewerServer allocate(SocketContext& p_ctx, const int32 p_port)
    {
        MaterialViewerServer l_return;
        l_return.socket_server = SocketServerSingleClient::allocate(p_ctx, p_port);
        l_return.engine_thread = EngineRunnerThread::allocate();
        return l_return;
    };

    inline void free(SocketContext& p_ctx)
    {
        this->socket_server.free(p_ctx);

        // TODO -> not needed because gracefully closed ?
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

    inline void wait_for_client(SocketContext& p_ctx)
    {
        this->socket_server.wait_for_client(p_ctx);
    };

    inline void listen_for_requests(SocketContext& p_ctx)
    {
        SocketRequestResponseConnection l_request_response_connection = SocketRequestResponseConnection::allocate_default();
        l_request_response_connection.listen(p_ctx, this->socket_server.registerd_client_socket, [&](const Slice<int8>& p_request, const Slice<int8>& p_response, uimax* out_response_size) {
            return this->handle_request(p_request, p_response, out_response_size);
        });

        if (!this->material_viewer_unit.is_freed())
        {
            this->material_viewer_unit.free(this->engine_thread);
        };
    };

    inline void mloop(SocketContext& p_ctx)
    {
        this->start_engine_thread();
        while (true)
        {
            this->wait_for_client(p_ctx);
            this->listen_for_requests(p_ctx);
        }
    };

  private:
    // TODO -> adding a variation that doesn't consume and return json ?
    inline SocketRequestResponseConnection::ListenSendResponseReturnCode handle_request(const Slice<int8>& p_request, const Slice<int8>& p_response, uimax* out_response_size)
    {
        SocketTypedRequest l_request = SocketTypedRequest::build(p_request);
        MaterialViewerRequestCode l_code = *(MaterialViewerRequestCode*)l_request.code;
        switch (l_code)
        {
        case MaterialViewerRequestCode::ENGINE_THREAD_START:
        {
            // this->engine_thread.start();
            this->material_viewer_unit = MaterialViewerEngineUnit::allocate();
            JSONDeserializer l_input_des = JSONDeserializer::start(l_request.payload);
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
        }
        break;
        case MaterialViewerRequestCode::SET_MATERIAL_AND_MESH:
        {
            JSONDeserializer l_input_des = JSONDeserializer::start(l_request.payload);

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

            JSONSerializer l_json_serializer = JSONSerializer::allocate_default();
            l_json_serializer.start();
            l_json_serializer.start_array(slice_int8_build_rawstr("materials"));
            for (loop(i, 0, l_material_paths.data.Size))
            {
                l_json_serializer.push_array_field(l_material_paths.data.get(i).slice);
            }
            l_json_serializer.end_array();
            l_json_serializer.end();

            l_material_paths.free();

            BinaryDeserializer l_payload_deserializer = BinaryDeserializer::build(l_request.payload);
            Slice<int8> l_heder = l_payload_deserializer.slice();

            SocketTypedResponse l_response = SocketTypedResponse::build(p_response);
            l_response.set_2((int32)MaterialViewerRequestCode::GET_ALL_MATERIALS_RETURN, l_heder, l_json_serializer.output.to_slice());
            *out_response_size = l_response.get_buffer_size();

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

            SocketTypedResponse l_response = SocketTypedResponse::build(p_response);
            l_response.set((int32)MaterialViewerRequestCode::GET_ALL_MESH_RETURN, l_json_deser.output.to_slice());
            *out_response_size = l_response.get_buffer_size();

            l_json_deser.free();
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

// TODO -> adding an ID to the request so that it can be retrieved to properly handle the response ?
struct MaterialViewerClient
{
    SocketClient client;
    SocketSendConnection client_sender;
    SocketRequestConnection response_listener;

    struct ResponseListenerClosure
    {
        int8 is_processed;
        int8* client_return_buffer;

        inline static ResponseListenerClosure build(int8* p_client_return_buffer)
        {
            return ResponseListenerClosure{0, p_client_return_buffer};
        };
    };

    Pool<ResponseListenerClosure> response_closures;

    inline static MaterialViewerClient allocate(SocketContext& p_ctx, const int32 p_port)
    {
        return MaterialViewerClient{SocketClient::allocate(p_ctx, p_port), SocketSendConnection::allocate_default(), SocketRequestConnection::allocate_default(),
                                    Pool<ResponseListenerClosure>::allocate(0)};
    };

    inline void free(SocketContext& p_ctx)
    {
#if __DEBUG
        assert_true(!this->response_closures.has_allocated_elements());
#endif

        this->client_sender.free();
        this->client.free(p_ctx);
        this->response_listener.free();
        this->response_closures.free();
    };

    inline void listen_for_responses(SocketContext& p_ctx)
    {
        this->response_listener.listen(p_ctx, this->client.client_socket, [this](const Slice<int8>& p_request_from_server) {
            return this->server_response_handler(p_request_from_server);
        });
    };

    inline void ENGINE_THREAD_START(const Slice<int8>& p_database)
    {
        JSONSerializer l_serializer = JSONSerializer::allocate_default();
        l_serializer.start();
        l_serializer.push_field(slice_int8_build_rawstr("database"), p_database);
        l_serializer.end();

        SocketTypedResponse l_request = SocketTypedResponse::build(this->client_sender.send_buffer.slice);
        l_request.set((int32)MaterialViewerRequestCode::ENGINE_THREAD_START, l_serializer.output.to_slice());

        this->client_sender.send(this->client.client_socket);

        l_serializer.free();
    };

    struct GetAllMaterialsReturn
    {
        Vector<Span<int8>> materials;

        inline static GetAllMaterialsReturn allocate_default()
        {
            return GetAllMaterialsReturn{Vector<Span<int8>>::allocate(0)};
        };

        inline void free()
        {
            for (loop(i, 0, this->materials.Size))
            {
                this->materials.get(i).free();
            }
            this->materials.free();
        };
    };

    inline GetAllMaterialsReturn GET_ALL_MATERIALS()
    {
        GetAllMaterialsReturn l_return;
        Token<ResponseListenerClosure> l_request_id = this->response_closures.alloc_element(ResponseListenerClosure::build((int8*)&l_return));

        SocketTypedResponse l_request = SocketTypedResponse::build(this->client_sender.send_buffer.slice);
        Span<uimax> l_payload = Span<uimax>::allocate(1);
        l_payload.get(0) = token_value(l_request_id);
        l_request.set((int32)MaterialViewerRequestCode::GET_ALL_MATERIALS, l_payload.slice.build_asint8());

        this->client_sender.send(this->client.client_socket);

        int8 l_is_processed = 0;
        while (!l_is_processed)
        {
            // TODO -> having a timer ?
            l_is_processed = this->response_closures.get(l_request_id).is_processed;
        }

        this->response_closures.release_element(l_request_id);
        l_payload.free();

        return l_return;
    };

  private:
    inline SocketRequestConnection::ListenSendResponseReturnCode server_response_handler(const Slice<int8>& p_request_from_server)
    {
        SocketTypedResponse l_typed_response = SocketTypedResponse::build(p_request_from_server);
        switch ((MaterialViewerRequestCode)*l_typed_response.code)
        {
        case MaterialViewerRequestCode::GET_ALL_MATERIALS_RETURN:
        {
            BinaryDeserializer l_payload_deserializer = BinaryDeserializer::build(l_typed_response.payload);
            Slice<int8> l_header = l_payload_deserializer.slice();
            Slice<int8> l_body = l_payload_deserializer.slice();

            BinaryDeserializer l_header_deserializer = BinaryDeserializer::build(l_header);
            Token<ResponseListenerClosure> l_request_id = *l_header_deserializer.type<Token<ResponseListenerClosure>>();

            ResponseListenerClosure& l_closure = this->response_closures.get(l_request_id);
            GetAllMaterialsReturn* l_get_all_materials_return = (GetAllMaterialsReturn*)l_closure.client_return_buffer;

            JSONDeserializer l_body_deserializer = JSONDeserializer::start(l_body);
            JSONDeserializer l_materials_array_deserializer = JSONDeserializer::allocate_default();
            l_body_deserializer.next_array("materials", &l_materials_array_deserializer);

            // TODO -> having a way to get the number of elements in a json array ?
            Vector<Span<int8>> l_materials = Vector<Span<int8>>::allocate(0);
            Slice<int8> l_material_str;
            while (l_materials_array_deserializer.next_array_plain_value(&l_material_str))
            {
                l_materials.push_back_element(Span<int8>::allocate_slice(l_material_str));
            }
            l_materials_array_deserializer.free();
            l_body_deserializer.free();

            l_get_all_materials_return->materials = l_materials;

            l_closure.is_processed = 1;

            break;
        }
        break;
        default:
            break;
        };

        return SocketRequestConnection::ListenSendResponseReturnCode::NOTHING;
    };
};