
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
    ENGINE_THREAD_START_RETURN = 8,
    ENGINE_THREAD_STOP = 1,
    ENGINE_THREAD_STOP_RETURN = 9,
    SET_MATERIAL_AND_MESH = 2,
    SET_MATERIAL_AND_MESH_RETURN = 10,
    GET_ALL_MATERIALS = 4,
    GET_ALL_MATERIALS_RETURN = 5,
    GET_ALL_MESH = 6,
    GET_ALL_MESH_RETURN = 7,
};

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

    struct EngineThreadStartInputJSON
    {
        Slice<int8> database;

        inline static void serialize(JSONSerializer& p_json, const Slice<int8>& p_database)
        {
            p_json.push_field(slice_int8_build_rawstr("database"), p_database);
        };

        inline static EngineThreadStartInputJSON deserialize(JSONDeserializer& p_json)
        {
            EngineThreadStartInputJSON l_return;
            p_json.next_field("database");
            l_return.database = p_json.get_currentfield().value;
            return l_return;
        };
    };

    struct GetAllMaterialsOutputJSON
    {
        Vector<Span<int8>> materials;

        inline void free()
        {
            for (loop(i, 0, this->materials.Size))
            {
                this->materials.get(i).free();
            }
            this->materials.free();
        };

        inline static GetAllMaterialsOutputJSON deserialize(JSONDeserializer& p_json)
        {
            GetAllMaterialsOutputJSON l_return;
            JSONDeserializer l_materials_array_deserializer = JSONDeserializer::allocate_default();
            p_json.next_array("materials", &l_materials_array_deserializer);

            // TODO -> having a way to get the number of elements in a json array ?
            l_return.materials = Vector<Span<int8>>::allocate(0);
            Slice<int8> l_material_str;
            while (l_materials_array_deserializer.next_array_plain_value(&l_material_str))
            {
                l_return.materials.push_back_element(Span<int8>::allocate_slice(l_material_str));
            }
            l_materials_array_deserializer.free();

            return l_return;
        };

        inline static void serialize_from_path(JSONSerializer& p_json, const AssetMetadataDatabase::Paths& p_paths)
        {
            p_json.start_array(slice_int8_build_rawstr("materials"));
            for (loop(i, 0, p_paths.data.Size))
            {
                p_json.push_array_field(p_paths.data.get(i).slice);
            }
            p_json.end_array();
        };
    };

    struct SetMaterialAndMeshInputJSON
    {
        Slice<int8> material;
        Slice<int8> mesh;

        inline static SetMaterialAndMeshInputJSON deserialize(JSONDeserializer& p_json)
        {
            SetMaterialAndMeshInputJSON l_return;
            p_json.next_field("material");
            l_return.material = p_json.get_currentfield().value;
            p_json.next_field("mesh");
            l_return.mesh = p_json.get_currentfield().value;
            return l_return;
        };

        inline void serialize(JSONSerializer& p_json) const
        {
            p_json.push_field(slice_int8_build_rawstr("material"), this->material);
            p_json.push_field(slice_int8_build_rawstr("mesh"), this->mesh);
        };
    };

    // TODO -> adding a variation that doesn't consume and return json ?
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
            EngineThreadStartInputJSON l_input = EngineThreadStartInputJSON::deserialize(l_input_des);

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

            SetMaterialAndMeshInputJSON l_input = SetMaterialAndMeshInputJSON::deserialize(l_input_des);

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
            GetAllMaterialsOutputJSON::serialize_from_path(l_json_serializer, l_material_paths);
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

            *in_out_response_slice = SocketTypedHeaderRequestWriter::set((int32)MaterialViewerRequestCode::GET_ALL_MESH_RETURN, l_request.header, l_json_deser.output.to_slice(), p_response);

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

struct MaterialViewerClient
{
    SocketSendConnection client_sender;
    SocketRequestResponseTracker response_tracker;

    inline static MaterialViewerClient allocate()
    {
        return MaterialViewerClient{SocketSendConnection::allocate_default(), SocketRequestResponseTracker::allocate()};
    };

    inline void free()
    {
        this->client_sender.free();
        this->response_tracker.free();
    };

    inline void ENGINE_THREAD_START(SocketContext& p_ctx, SocketClient& p_client, const Slice<int8>& p_database)
    {
        Token<SocketRequestResponseTracker::ResponseListenerClosure> l_request_id =
            this->response_tracker.response_closures.alloc_element(SocketRequestResponseTracker::ResponseListenerClosure::build(NULL));

        JSONSerializer l_serializer = JSONSerializer::allocate_default();
        l_serializer.start();
        MaterialViewerServerV2::EngineThreadStartInputJSON::serialize(l_serializer, p_database);
        l_serializer.end();

        Slice<int8> l_written_slice = SocketTypedHeaderRequestWriter::set((int32)MaterialViewerRequestCode::ENGINE_THREAD_START,
                                                                          Slice<Token<SocketRequestResponseTracker::ResponseListenerClosure>>::build_asint8_memory_singleelement(&l_request_id),
                                                                          l_serializer.output.to_slice(), this->client_sender.send_buffer.slice);

        this->client_sender.send_v2(p_ctx, p_client.client_socket, l_written_slice);

        this->response_tracker.wait_for_request_completion(l_request_id);

        l_serializer.free();
    };

    inline void ENGINE_THREAD_STOP(SocketContext& p_ctx, SocketClient& p_client)
    {
        Token<SocketRequestResponseTracker::ResponseListenerClosure> l_request_id =
            this->response_tracker.response_closures.alloc_element(SocketRequestResponseTracker::ResponseListenerClosure::build(NULL));

        Slice<int8> l_written_slice = SocketTypedHeaderRequestWriter::set_code_header(
            (int32)MaterialViewerRequestCode::ENGINE_THREAD_STOP, Slice<Token<SocketRequestResponseTracker::ResponseListenerClosure>>::build_asint8_memory_singleelement(&l_request_id),
            this->client_sender.send_buffer.slice);

        this->client_sender.send_v2(p_ctx, p_client.client_socket, l_written_slice);

        this->response_tracker.wait_for_request_completion(l_request_id);
    };

    inline void SET_MATERIAL_AND_MESH(SocketContext& p_ctx, SocketClient& p_client, const MaterialViewerServerV2::SetMaterialAndMeshInputJSON& p_input)
    {
        Token<SocketRequestResponseTracker::ResponseListenerClosure> l_request_id =
            this->response_tracker.response_closures.alloc_element(SocketRequestResponseTracker::ResponseListenerClosure::build(NULL));

        JSONSerializer l_serializer = JSONSerializer::allocate_default();
        l_serializer.start();
        p_input.serialize(l_serializer);
        l_serializer.end();

        Slice<int8> l_written_slice = SocketTypedHeaderRequestWriter::set((int32)MaterialViewerRequestCode::SET_MATERIAL_AND_MESH,
                                                                          Slice<Token<SocketRequestResponseTracker::ResponseListenerClosure>>::build_asint8_memory_singleelement(&l_request_id),
                                                                          l_serializer.output.to_slice(), this->client_sender.send_buffer.slice);

        this->client_sender.send_v2(p_ctx, p_client.client_socket, l_written_slice);

        this->response_tracker.wait_for_request_completion(l_request_id);

        l_serializer.free();
    };

    inline MaterialViewerServerV2::GetAllMaterialsOutputJSON GET_ALL_MATERIALS(SocketContext& p_ctx, SocketClient& p_client)
    {
        MaterialViewerServerV2::GetAllMaterialsOutputJSON l_return;
        Token<SocketRequestResponseTracker::ResponseListenerClosure> l_request_id =
            this->response_tracker.response_closures.alloc_element(SocketRequestResponseTracker::ResponseListenerClosure::build((int8*)&l_return));

        Slice<int8> l_written_slice = SocketTypedHeaderRequestWriter::set_code_header(
            (int32)MaterialViewerRequestCode::GET_ALL_MATERIALS, Slice<Token<SocketRequestResponseTracker::ResponseListenerClosure>>::build_asint8_memory_singleelement(&l_request_id),
            this->client_sender.send_buffer.slice);

        this->client_sender.send_v2(p_ctx, p_client.client_socket, l_written_slice);

        this->response_tracker.wait_for_request_completion(l_request_id);

        return l_return;
    };

    inline void server_response_handler_json(const Slice<int8>& p_request_from_server)
    {
        SocketTypedHeaderRequestReader l_request = SocketTypedHeaderRequestReader::build(p_request_from_server);
        switch ((MaterialViewerRequestCode)*l_request.code)
        {
        case MaterialViewerRequestCode::ENGINE_THREAD_START_RETURN:
        {
            BinaryDeserializer l_header_deserializer = BinaryDeserializer::build(l_request.header);
            Token<SocketRequestResponseTracker::ResponseListenerClosure> l_request_id = *l_header_deserializer.type<Token<SocketRequestResponseTracker::ResponseListenerClosure>>();

            this->response_tracker.set_request_is_completed(l_request_id);
        }
        break;
        case MaterialViewerRequestCode::ENGINE_THREAD_STOP_RETURN:
        {
            BinaryDeserializer l_header_deserializer = BinaryDeserializer::build(l_request.header);
            Token<SocketRequestResponseTracker::ResponseListenerClosure> l_request_id = *l_header_deserializer.type<Token<SocketRequestResponseTracker::ResponseListenerClosure>>();

            this->response_tracker.set_request_is_completed(l_request_id);
        }
        break;
        case MaterialViewerRequestCode::SET_MATERIAL_AND_MESH_RETURN:
        {
            BinaryDeserializer l_header_deserializer = BinaryDeserializer::build(l_request.header);
            Token<SocketRequestResponseTracker::ResponseListenerClosure> l_request_id = *l_header_deserializer.type<Token<SocketRequestResponseTracker::ResponseListenerClosure>>();

            this->response_tracker.set_request_is_completed(l_request_id);
        }
        break;
        case MaterialViewerRequestCode::GET_ALL_MATERIALS_RETURN:
        {
            BinaryDeserializer l_header_deserializer = BinaryDeserializer::build(l_request.header);
            Token<SocketRequestResponseTracker::ResponseListenerClosure> l_request_id = *l_header_deserializer.type<Token<SocketRequestResponseTracker::ResponseListenerClosure>>();

            SocketRequestResponseTracker::ResponseListenerClosure& l_closure = this->response_tracker.response_closures.get(l_request_id);
            MaterialViewerServerV2::GetAllMaterialsOutputJSON* l_get_all_materials_return = (MaterialViewerServerV2::GetAllMaterialsOutputJSON*)l_closure.client_return_buffer;

            JSONDeserializer l_body_deserializer = JSONDeserializer::start(l_request.body);
            *l_get_all_materials_return = MaterialViewerServerV2::GetAllMaterialsOutputJSON::deserialize(l_body_deserializer);
            l_body_deserializer.free();

            l_closure.is_processed = 1;
        }
        break;
        default:
            break;
        };
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

struct MaterialViewerClientJSONThread
{
    struct Exec
    {
        MaterialViewerClientJSONThread* thiz;
        inline void operator()() const
        {
            thiz->thread.client.listen_request(*thiz->thread.input.ctx, [&](const Slice<int8>& p_request) {
                thiz->material_viewer_client.server_response_handler_json(p_request);
            });
        };
    } exec;

    SocketClientThread<Exec> thread;
    MaterialViewerClient material_viewer_client;

    inline void start(SocketContext* p_ctx, const int32 p_port)
    {
        this->exec = Exec{this};
        this->material_viewer_client = MaterialViewerClient::allocate();
        this->thread.start(p_ctx, p_port, &this->exec);
    };

    inline void free(SocketContext& p_ctx)
    {
        this->thread.free(p_ctx);
        this->material_viewer_client.free();
    };
};