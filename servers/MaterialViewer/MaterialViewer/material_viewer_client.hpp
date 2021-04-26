#pragma once


enum class MaterialViewerRequestCode : int32
{
    ENGINE_THREAD_START = 0,
    ENGINE_THREAD_START_RETURN = 1,
    ENGINE_THREAD_STOP = 2,
    ENGINE_THREAD_STOP_RETURN = 3,
    SET_MATERIAL_AND_MESH = 4,
    SET_MATERIAL_AND_MESH_RETURN = 5,
    GET_ALL_MATERIALS = 6,
    GET_ALL_MATERIALS_RETURN = 7,
    GET_ALL_MESH = 8,
    GET_ALL_MESH_RETURN = 9,
};

struct MaterialViewerClient
{
    SocketRequestResponseTracker response_tracker;

    inline static MaterialViewerClient allocate()
    {
        return MaterialViewerClient{SocketRequestResponseTracker::allocate()};
    };

    inline void free()
    {
        this->response_tracker.free();
    };

    inline void ENGINE_THREAD_START(SocketContext& p_ctx, SocketClient& p_client, const Slice<int8>& p_database)
    {
        Token<SocketRequestResponseTracker::Response> l_request_id =
            this->response_tracker.response_closures.alloc_element(SocketRequestResponseTracker::Response::build(Slice<int8>::build_default()));

        JSONSerializer l_serializer = JSONSerializer::allocate_default();
        l_serializer.start();
        MaterialViewerDomain::EngineThreadStartInputJSON::serialize(l_serializer, p_database);
        l_serializer.end();

        Slice<int8> l_written_slice = SocketTypedHeaderRequestWriter::set((int32)MaterialViewerRequestCode::ENGINE_THREAD_START,
                                                                          Slice<Token<SocketRequestResponseTracker::Response>>::build_asint8_memory_singleelement(&l_request_id),
                                                                          l_serializer.output.to_slice(), p_client.get_client_to_server_buffer());

        p_client.send_client_to_server(p_ctx, l_written_slice);

        this->response_tracker.wait_for_request_completion(l_request_id);

        l_serializer.free();
    };

    inline void ENGINE_THREAD_STOP(SocketContext& p_ctx, SocketClient& p_client)
    {
        Token<SocketRequestResponseTracker::Response> l_request_id =
            this->response_tracker.response_closures.alloc_element(SocketRequestResponseTracker::Response::build(Slice<int8>::build_default()));

        Slice<int8> l_written_slice = SocketTypedHeaderRequestWriter::set_code_header((int32)MaterialViewerRequestCode::ENGINE_THREAD_STOP,
                                                                                      Slice<Token<SocketRequestResponseTracker::Response>>::build_asint8_memory_singleelement(&l_request_id),
                                                                                      p_client.get_client_to_server_buffer());

        p_client.send_client_to_server(p_ctx, l_written_slice);

        this->response_tracker.wait_for_request_completion(l_request_id);
    };

    inline void SET_MATERIAL_AND_MESH(SocketContext& p_ctx, SocketClient& p_client, const MaterialViewerDomain::SetMaterialAndMeshInputJSON& p_input)
    {
        Token<SocketRequestResponseTracker::Response> l_request_id =
            this->response_tracker.response_closures.alloc_element(SocketRequestResponseTracker::Response::build(Slice<int8>::build_default()));

        JSONSerializer l_serializer = JSONSerializer::allocate_default();
        l_serializer.start();
        p_input.serialize(l_serializer);
        l_serializer.end();

        Slice<int8> l_written_slice = SocketTypedHeaderRequestWriter::set((int32)MaterialViewerRequestCode::SET_MATERIAL_AND_MESH,
                                                                          Slice<Token<SocketRequestResponseTracker::Response>>::build_asint8_memory_singleelement(&l_request_id),
                                                                          l_serializer.output.to_slice(), p_client.get_client_to_server_buffer());

        p_client.send_client_to_server(p_ctx, l_written_slice);

        this->response_tracker.wait_for_request_completion(l_request_id);

        l_serializer.free();
    };

    inline MaterialViewerDomain::GetAllMaterialsOutputJSON GET_ALL_MATERIALS(SocketContext& p_ctx, SocketClient& p_client)
    {
        MaterialViewerDomain::GetAllMaterialsOutputJSON l_return;
        Token<SocketRequestResponseTracker::Response> l_request_id = this->response_tracker.response_closures.alloc_element(
            SocketRequestResponseTracker::Response::build(Slice<MaterialViewerDomain::GetAllMaterialsOutputJSON>::build_asint8_memory_singleelement(&l_return)));

        Slice<int8> l_written_slice = SocketTypedHeaderRequestWriter::set_code_header((int32)MaterialViewerRequestCode::GET_ALL_MATERIALS,
                                                                                      Slice<Token<SocketRequestResponseTracker::Response>>::build_asint8_memory_singleelement(&l_request_id),
                                                                                      p_client.get_client_to_server_buffer());

        p_client.send_client_to_server(p_ctx, l_written_slice);

        this->response_tracker.wait_for_request_completion(l_request_id);

        return l_return;
    };

    inline MaterialViewerDomain::GetAllMeshesOutputJSON GET_ALL_MESH(SocketContext& p_ctx, SocketClient& p_client)
    {
        MaterialViewerDomain::GetAllMeshesOutputJSON l_return;
        Token<SocketRequestResponseTracker::Response> l_request_id = this->response_tracker.response_closures.alloc_element(
            SocketRequestResponseTracker::Response::build(Slice<MaterialViewerDomain::GetAllMeshesOutputJSON>::build_asint8_memory_singleelement(&l_return)));

        Slice<int8> l_written_slice = SocketTypedHeaderRequestWriter::set_code_header((int32)MaterialViewerRequestCode::GET_ALL_MESH,
                                                                                      Slice<Token<SocketRequestResponseTracker::Response>>::build_asint8_memory_singleelement(&l_request_id),
                                                                                      p_client.get_client_to_server_buffer());

        p_client.send_client_to_server(p_ctx, l_written_slice);

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
            Token<SocketRequestResponseTracker::Response> l_request_id = *l_header_deserializer.type<Token<SocketRequestResponseTracker::Response>>();

            this->response_tracker.set_request_is_completed(l_request_id);
        }
            break;
        case MaterialViewerRequestCode::ENGINE_THREAD_STOP_RETURN:
        {
            BinaryDeserializer l_header_deserializer = BinaryDeserializer::build(l_request.header);
            Token<SocketRequestResponseTracker::Response> l_request_id = *l_header_deserializer.type<Token<SocketRequestResponseTracker::Response>>();

            this->response_tracker.set_request_is_completed(l_request_id);
        }
            break;
        case MaterialViewerRequestCode::SET_MATERIAL_AND_MESH_RETURN:
        {
            BinaryDeserializer l_header_deserializer = BinaryDeserializer::build(l_request.header);
            Token<SocketRequestResponseTracker::Response> l_request_id = *l_header_deserializer.type<Token<SocketRequestResponseTracker::Response>>();

            this->response_tracker.set_request_is_completed(l_request_id);
        }
            break;
        case MaterialViewerRequestCode::GET_ALL_MATERIALS_RETURN:
        {
            BinaryDeserializer l_header_deserializer = BinaryDeserializer::build(l_request.header);
            Token<SocketRequestResponseTracker::Response> l_request_id = *l_header_deserializer.type<Token<SocketRequestResponseTracker::Response>>();

            JSONDeserializer l_body_deserializer = JSONDeserializer::start(l_request.body);
            MaterialViewerDomain::GetAllMaterialsOutputJSON l_get_all_materials_return = MaterialViewerDomain::GetAllMaterialsOutputJSON::deserialize(l_body_deserializer);
            l_body_deserializer.free();

            this->response_tracker.set_response_slice(l_request_id, Slice<MaterialViewerDomain::GetAllMaterialsOutputJSON>::build_asint8_memory_singleelement(&l_get_all_materials_return));
            this->response_tracker.set_request_is_completed(l_request_id);
        }
            break;
        case MaterialViewerRequestCode::GET_ALL_MESH_RETURN:
        {
            BinaryDeserializer l_header_deserializer = BinaryDeserializer::build(l_request.header);
            Token<SocketRequestResponseTracker::Response> l_request_id = *l_header_deserializer.type<Token<SocketRequestResponseTracker::Response>>();

            JSONDeserializer l_body_deserializer = JSONDeserializer::start(l_request.body);
            MaterialViewerDomain::GetAllMeshesOutputJSON l_get_all_meshes_return = MaterialViewerDomain::GetAllMeshesOutputJSON::deserialize(l_body_deserializer);
            l_body_deserializer.free();

            this->response_tracker.set_response_slice(l_request_id, Slice<MaterialViewerDomain::GetAllMeshesOutputJSON>::build_asint8_memory_singleelement(&l_get_all_meshes_return));
            this->response_tracker.set_request_is_completed(l_request_id);
        }
            break;
        default:
            break;
        };
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
