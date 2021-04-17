
#include "Engine/engine.hpp"
#include "AssetCompiler/asset_compiler.hpp"

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
// #include <bcrypt.h>
#pragma comment(lib, "Ws2_32.lib")

template <class ReturnType> inline ReturnType winsock_error_handler(const ReturnType p_return)
{
#if __DEBUG
    if (p_return != 0)
    {
        abort();
    }
#endif
    return p_return;
};

#if 0
inline Span<int8> sha1_hash(const Slice<int8>& p_input){
    BCRYPT_ALG_HANDLE hAlg = NULL;
    BCRYPT_HASH_HANDLE hHash = NULL;
    DWORD cbData = 0, cbHash = 0;
    PBYTE pbHashObject = NULL;
    PBYTE pbHash = NULL;

    Span<int8> l_hash_object;
    Span<int8> l_hash;

    // open an algorithm handle
    winsock_error_handler(BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_SHA256_ALGORITHM, NULL, 0));

    // calculate the size of the buffer to hold the hash object
    winsock_error_handler(BCryptGetProperty(hAlg, BCRYPT_OBJECT_LENGTH, (PBYTE)&l_hash_object.Capacity, sizeof(DWORD), &cbData, 0));

    // allocate the hash object on the heap
    l_hash_object = Span<int8>::allocate(l_hash_object.Capacity);

    // calculate the length of the hash
    winsock_error_handler(BCryptGetProperty(hAlg, BCRYPT_HASH_LENGTH, (PBYTE)&l_hash.Capacity, sizeof(DWORD), &cbData, 0));
   
    // allocate the hash buffer on the heap
    l_hash = Span<int8>::allocate(l_hash.Capacity);

    // create a hash
    winsock_error_handler(BCryptCreateHash(hAlg, &hHash, pbHashObject, (ULONG)l_hash_object.Memory, NULL, 0, 0));

#if 0
    // hash some data
    if (!NT_SUCCESS(status = BCryptHashData(hHash, (PBYTE)rgbMsg, sizeof(rgbMsg), 0)))
    {
        wprintf(L"**** Error 0x%x returned by BCryptHashData\n", status);
        goto Cleanup;
    }

    // close the hash
    if (!NT_SUCCESS(status = BCryptFinishHash(hHash, pbHash, cbHash, 0)))
    {
        wprintf(L"**** Error 0x%x returned by BCryptFinishHash\n", status);
        goto Cleanup;
    }
#endif

    l_hash_object.free();
    BCryptCloseAlgorithmProvider(hAlg, 0);
    BCryptDestroyHash(hHash);

    return l_hash;
};
#endif

using socket_t = SOCKET;

struct SocketServer
{
    socket_t client_listening_socket;
    socket_t client_socket;

    inline static SocketServer allocate(const int32 p_port)
    {
        SocketServer l_return;
        int8 p_port_str_raw[ToString::int32str_size];
        ToString::aint32(p_port, slice_int8_build_rawstr(p_port_str_raw));

        addrinfo l_addr_info{};
        l_addr_info.ai_family = AF_INET;
        l_addr_info.ai_socktype = SOCK_STREAM;
        l_addr_info.ai_protocol = IPPROTO_TCP;
        l_addr_info.ai_flags = AI_PASSIVE;

        addrinfo* l_result = NULL;

        winsock_error_handler(getaddrinfo(NULL, p_port_str_raw, &l_addr_info, &l_result));

        l_return.client_listening_socket = INVALID_SOCKET;
        l_return.client_listening_socket = socket(l_result->ai_family, l_result->ai_socktype, l_result->ai_protocol);
        assert_true(l_return.client_listening_socket != INVALID_SOCKET);

        winsock_error_handler(bind(l_return.client_listening_socket, l_result->ai_addr, (int)l_result->ai_addrlen));

        freeaddrinfo(l_result);

        return l_return;
    };

    inline void wait_for_client()
    {
        winsock_error_handler(listen(this->client_listening_socket, SOMAXCONN));

        this->client_socket = INVALID_SOCKET;
        this->client_socket = accept(this->client_listening_socket, NULL, NULL);
        assert_true(this->client_socket != INVALID_SOCKET);
    };

    template <class RequestCallbackFunc> inline void listen_for_requests(const RequestCallbackFunc& p_request_callback_func)
    {
        SliceN<int8, 512> l_buffer = {};
        Slice<int8> l_buffer_slice = slice_from_slicen(&l_buffer);
        SliceN<int8, 512> l_response_buffer = {};
        Slice<int8> l_response_slice = slice_from_slicen(&l_response_buffer);
        int32 l_result = 1, l_send_result;

        while (l_result > 0)
        {
            l_result = recv(this->client_socket, l_buffer.Memory, (int)l_buffer.Size(), 0);
            if (l_result > 0)
            {
                p_request_callback_func(l_buffer_slice, l_response_slice);
                l_send_result = send(this->client_socket, l_response_slice.Begin, (int)l_response_slice.Size, 0);
#if 0
                if (l_buffer_slice.compare(slice_int8_build_rawstr("GET /")))
                {
                    uimax l_index;
                    if (Slice_find(l_buffer_slice, slice_int8_build_rawstr("Upgrade: websocket"), &l_index) && Slice_find(l_buffer_slice, slice_int8_build_rawstr("Connection: Upgrade"), &l_index))
                    {
                        const int8* l_sec_key_header_prefix_raw = "Sec-WebSocket-Key: ";
                        Slice<int8> l_sec_key_header_prefix = slice_int8_build_rawstr("Sec-WebSocket-Key: ");
                        if (Slice_find(l_buffer_slice, l_sec_key_header_prefix, &l_index))
                        {
                            Slice<int8> l_key = l_buffer_slice.slide_rv(l_index + l_sec_key_header_prefix.Size);
                            if (Slice_find(l_key, slice_int8_build_rawstr("\r\n"), &l_index))
                            {
                                l_key.Size = l_index;
                            }

                            // Span<int8> l_hashed_key = sha1_hash(l_key);
                            
                            String l_http_request = String::allocate_elements_3(slice_int8_build_rawstr("HTTP/1.1 101 Switching Protocols\r\nUpgrade : websocket\r\nConnection : Upgrade\r\nSec-WebSocket-Accept : "),
                                                                                l_key, slice_int8_build_rawstr("\r\n"));
                            l_buffer_slice.Begin = l_http_request.to_slice().Begin;
                            l_send_result = send(this->client_socket, l_buffer.Memory, l_buffer.Size(), 0);

                            l_http_request.free(); 
                            // l_hashed_key.free();
                        }

                        
                    }
                }
#endif
            }
            else if (l_result <= 0)
            {
                closesocket(this->client_socket);
            }
        }
    };
};

struct SocketRequest
{
    int32 code;
    Slice<int8> payload;

    inline static SocketRequest build(const Slice<int8>& p_request)
    {
        SocketRequest l_return;
        BinaryDeserializer l_binary_desezrialiazer = BinaryDeserializer::build(p_request);
        l_return.code = *l_binary_desezrialiazer.type<int32>();
        l_return.payload = l_binary_desezrialiazer.memory;
        return l_return;
    };
};

enum class MaterialViewerRequestCode : int32
{
    ENGINE_THREAD_START = 0,
    ENGINE_THREAD_STOP = 1,
    SET_MATERIAL_AND_MESH = 2,
    GET_ALL_MESHES = 3,
    GET_ALL_MATERIALS = 4,
    GET_ALL_MATERIALS_RETURN = 5
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

    inline static void listen(const int32 p_port)
    {
        MaterialViewerServer thiz{};
        thiz._listen(p_port);
    };

  private:
    inline void _listen(const int32 p_port)
    {
        WSADATA l_winsock_data;
        winsock_error_handler(WSAStartup(MAKEWORD(2, 2), &l_winsock_data));

        this->socket_server = SocketServer::allocate(8000);
        this->socket_server.wait_for_client();
        this->socket_server.listen_for_requests([&](const Slice<int8>& p_request, Slice<int8>& p_response) {
            SocketRequest l_request = SocketRequest::build(p_request);
            MaterialViewerRequestCode l_request_code = (MaterialViewerRequestCode)l_request.code;
            p_response.zero();
            this->handle_request(l_request_code, l_request.payload, p_response);
        });
    };

    // TODO -> we want to return JSON format so that it can be easily undertable by client.
    inline void handle_request(const MaterialViewerRequestCode p_code, const Slice<int8>& p_payload, const Slice<int8>& p_response)
    {
        switch (p_code)
        {
        case MaterialViewerRequestCode::ENGINE_THREAD_START:
        {
            this->engine_thread.start();
            this->material_viewer_unit = MaterialViewerEngineUnit::allocate();

            BinaryDeserializer l_payload_deserializer = BinaryDeserializer::build(p_payload);
            Slice<int8> l_database_path = l_payload_deserializer.slice();
            this->material_viewer_unit.start(this->engine_thread, l_database_path, 400, 400);
            this->engine_thread.sync_wait_for_engine_execution_unit_to_be_allocated(this->material_viewer_unit.engine_execution_unit);

            Slice<int8> l_response = p_response;
            BinarySerializer::type<int32>(&l_response, 1);
        }
        break;
        case MaterialViewerRequestCode::ENGINE_THREAD_STOP:
        {
            this->material_viewer_unit.stop(this->engine_thread);
            this->engine_thread.free();

            Slice<int8> l_response = p_response;
            BinarySerializer::type<int32>(&l_response, 1);
        }
        break;
        case MaterialViewerRequestCode::SET_MATERIAL_AND_MESH:
        {
            BinaryDeserializer l_payload_deserializer = BinaryDeserializer::build(p_payload);
            Slice<int8> l_material = l_payload_deserializer.slice();
            Slice<int8> l_mesh = l_payload_deserializer.slice();

            this->material_viewer_unit.set_new_material(HashSlice(l_material));
            this->material_viewer_unit.set_new_mesh(HashSlice(l_mesh));

            Slice<int8> l_response = p_response;
            BinarySerializer::type<int32>(&l_response, 1);
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
        }
        break;
        default:
            break;
        }
    };
};

int main()
{
    MaterialViewerServer::listen(8000);
    return 0;
};