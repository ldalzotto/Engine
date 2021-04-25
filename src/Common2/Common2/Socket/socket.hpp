#pragma once

#include <winsock2.h>
#include <ws2tcpip.h>
// #include <iphlpapi.h>
// #include <bcrypt.h>
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")
#pragma comment(lib, "AdvApi32.lib")

#define SOCKET_DEFAULT_PORT 8000

template <class ReturnType> inline ReturnType winsock_error_handler(const ReturnType p_return)
{
#if __DEBUG
    if (p_return != 0)
    {
        int l_last_error = WSAGetLastError();
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

struct SocketContext
{
    MutexNative<Vector<socket_t>> sockets;

    inline static SocketContext allocate()
    {
        WSADATA l_winsock_data;
        winsock_error_handler(WSAStartup(MAKEWORD(2, 2), &l_winsock_data));

        SocketContext l_ctx;
        l_ctx.sockets = MutexNative<Vector<socket_t>>::allocate();
        l_ctx.sockets._data = Vector<socket_t>::allocate(0);
        return l_ctx;
    };

    inline void free()
    {
        this->sockets.acquire([](Vector<socket_t>& p_sockets) {
#if __DEBUG
            assert_true(p_sockets.empty());
#endif
            p_sockets.free();
        });
        this->sockets.free();
        WSACleanup();
    };

    inline void remove_socket(const socket_t p_socket)
    {
        this->sockets.acquire([&](Vector<socket_t>& p_sockets) {
            for (loop(i, 0, p_sockets.Size))
            {
                if (p_sockets.get(i) == p_socket)
                {
                    p_sockets.erase_element_at_always(i);
                    break;
                }
            }
        });
    };
};

inline socket_t socket_allocate(SocketContext& p_ctx, const socket_t p_socket)
{
#if __DEBUG
    assert_true(p_socket != INVALID_SOCKET);
#endif
#if __MEMLEAK
    push_ptr_to_tracked((int8*)p_socket);
#endif
    p_ctx.sockets.acquire([&](Vector<socket_t>& p_sockets) {
        p_sockets.push_back_element(p_socket);
    });

    return p_socket;
};

inline void socket_close(SocketContext& p_ctx, socket_t* p_socket)
{
#if __MEMLEAK
    remove_ptr_to_tracked((int8*)*p_socket);
#endif
#if __DEBUG
    assert_true(*p_socket != INVALID_SOCKET);
#endif
    closesocket(*p_socket);
    p_ctx.remove_socket(*p_socket);
    *p_socket = INVALID_SOCKET;
};

inline void socket_shutdown(SocketContext& p_ctx, socket_t* p_socket)
{
#if __DEBUG
    assert_true(*p_socket != INVALID_SOCKET);
#endif
    // TODO -> gracefully shutdown
    HANDLE l_wsa_event = WSACreateEvent();
    WSAEventSelect(*p_socket, l_wsa_event, FD_CLOSE);
    shutdown(*p_socket, SD_BOTH);
    WSAWaitForMultipleEvents(1, &l_wsa_event, TRUE, WSA_INFINITE, TRUE);
    // TODO -> release event ?
};

enum class SocketReturnCode
{
    IDLING = 0,
    GRACEFULLY_CLOSED = 1
};

// TODO -> handling disconnect
inline SocketReturnCode socket_receive(socket_t p_socket, int8* p_begin, const uimax p_size)
{
    int l_result = ::recv(p_socket, p_begin, (int)p_size, 0);
#if __DEBUG
    assert_true(l_result != SOCKET_ERROR);
#endif

    if (l_result == 0)
    {
        return SocketReturnCode::GRACEFULLY_CLOSED;
    }
    return SocketReturnCode::IDLING;
};

inline SocketReturnCode socket_send(socket_t p_socket, const int8* p_begin, const uimax p_size)
{
    int l_result = ::send(p_socket, p_begin, (int)(p_size), 0);
#if __DEBUG
    assert_true(l_result != SOCKET_ERROR);
#endif

    if (l_result == 0)
    {
        return SocketReturnCode::GRACEFULLY_CLOSED;
    }
    return SocketReturnCode::IDLING;
};

struct Socket
{
    socket_t native_socket;
    addrinfo* native_addr_info;

    inline static Socket allocate_as_server(SocketContext& p_ctx, const int32 p_port)
    {
        addrinfo l_addr_info{};
        l_addr_info.ai_family = AF_INET;
        l_addr_info.ai_socktype = SOCK_STREAM;
        l_addr_info.ai_protocol = IPPROTO_TCP;
        l_addr_info.ai_flags = AI_PASSIVE;

        Socket l_socket;
        _allocate_socket_and_bind(l_socket, p_ctx, l_addr_info, p_port);
        return l_socket;
    };

    inline static Socket allocate_as_client(SocketContext& p_ctx, const int32 p_port)
    {
        addrinfo l_addr_info{};
        l_addr_info.ai_family = AF_INET;
        l_addr_info.ai_socktype = SOCK_STREAM;
        l_addr_info.ai_protocol = IPPROTO_TCP;

        Socket l_socket;
        _allocate_socket_and_connect(l_socket, p_ctx, l_addr_info, p_port);
        return l_socket;
    };

    inline static Socket allocate_as_request_listener(SocketContext& p_ctx, const Socket p_listened_socket)
    {
        Socket l_socket;
        l_socket.native_socket = socket_allocate(p_ctx, accept(p_listened_socket.native_socket, NULL, NULL));
        l_socket.native_addr_info = NULL;
        return l_socket;
    };

    inline void free_gracefully(SocketContext& p_ctx)
    {
        freeaddrinfo(this->native_addr_info);
        this->native_addr_info = NULL;
        socket_shutdown(p_ctx, &this->native_socket);
    };

    inline void abort(SocketContext& p_ctx)
    {
        freeaddrinfo(this->native_addr_info);
        this->native_addr_info = NULL;
        socket_close(p_ctx, &this->native_socket);
    };

    inline int8 is_freed()
    {
        return this->native_socket == INVALID_SOCKET;
    };

    inline SocketReturnCode send(const Slice<int8>& p_buffer)
    {
        return socket_send(this->native_socket, p_buffer.Begin, p_buffer.Size);
    };

  private:
    inline static void _allocate_socket_and_bind(Socket& p_socket, SocketContext& p_ctx, const addrinfo& p_addr_info, const int32 p_port)
    {
        int8 p_port_str_raw[ToString::int32str_size];
        ToString::aint32(p_port, slice_int8_build_rawstr(p_port_str_raw));

        p_socket.native_addr_info = NULL;
        winsock_error_handler(getaddrinfo(NULL, p_port_str_raw, &p_addr_info, &p_socket.native_addr_info));

        p_socket.native_socket = socket_allocate(p_ctx, socket(p_socket.native_addr_info->ai_family, p_socket.native_addr_info->ai_socktype, p_socket.native_addr_info->ai_protocol));
        winsock_error_handler(bind(p_socket.native_socket, p_socket.native_addr_info->ai_addr, (int)p_socket.native_addr_info->ai_addrlen));
    };

    inline static void _allocate_socket_and_connect(Socket& p_socket, SocketContext& p_ctx, const addrinfo& p_addr_info, const int32 p_port)
    {
        int8 p_port_str_raw[ToString::int32str_size];
        ToString::aint32(p_port, slice_int8_build_rawstr(p_port_str_raw));

        p_socket.native_addr_info = NULL;
        winsock_error_handler(getaddrinfo(NULL, p_port_str_raw, &p_addr_info, &p_socket.native_addr_info));

        addrinfo* l_ptr;
        addrinfo* l_result = p_socket.native_addr_info;
        for (l_ptr = l_result; l_ptr != NULL; l_ptr = l_ptr->ai_next)
        {
            p_socket.native_socket = socket_allocate(p_ctx, socket(l_result->ai_family, l_result->ai_socktype, l_result->ai_protocol));
            if (connect(p_socket.native_socket, l_result->ai_addr, (int)l_result->ai_addrlen) != 0)
            {
                // TODO -> force
                socket_shutdown(p_ctx, &p_socket.native_socket);
                continue;
            }
            else
            {
                break;
            }
        };
        if (l_ptr == NULL)
        {
            ::abort();
        }
    };
};

struct SocketServerSingleClient
{
    int32 port;
    Socket server_socket;
    Socket registerd_client_socket;

    inline static SocketServerSingleClient allocate(SocketContext& p_ctx, const int32 p_port)
    {
        return SocketServerSingleClient{p_port};
    };

    inline void free(SocketContext& p_ctx)
    {
        if (!this->server_socket.is_freed())
        {
            this->server_socket.abort(p_ctx);
        }

#if __DEBUG
        assert_true(this->server_socket.is_freed());
        assert_true(this->registerd_client_socket.is_freed());
#endif
    };

    inline void wait_for_client(SocketContext& p_ctx)
    {
        this->server_socket = Socket::allocate_as_server(p_ctx, this->port);
        if (listen(this->server_socket.native_socket, SOMAXCONN) == 0)
        {
            // TODO -> bug, the listen return 0 when we close the listening socket ?
            this->registerd_client_socket = Socket::allocate_as_request_listener(p_ctx, this->server_socket);
            this->server_socket.abort(p_ctx);
        }
        else
        {
#if __DEBUG
            assert_true(this->server_socket.is_freed());
#endif
        }
    };
};

template <class SocketConncetionEstablishmentFunc> struct SocketSocketServerSingleClientThread
{

    struct Input
    {
        SocketContext* ctx;
        int32 port;
    } input;
    thread_t thread;

    struct Sync
    {
        volatile int8 allocated;
    } sync;

    struct Exec
    {
        SocketSocketServerSingleClientThread<SocketConncetionEstablishmentFunc>* thiz;
        inline int8 operator()() const
        {
            return SocketSocketServerSingleClientThread<SocketConncetionEstablishmentFunc>::main(thiz);
        };
    } exec;

    SocketServerSingleClient server;
    int8 ask_end_thread;
    const SocketConncetionEstablishmentFunc* socket_connection_establishment_func;

    inline void start(SocketContext* p_ctx, const int32 p_port, const SocketConncetionEstablishmentFunc* p_socket_connection_establishment_func)
    {
        this->exec = Exec{this};
        this->input = {p_ctx, p_port};
        this->sync.allocated = 0;
        this->ask_end_thread = 0;
        this->socket_connection_establishment_func = p_socket_connection_establishment_func;
        this->thread = Thread::spawn_thread(this->exec);
    };

    inline void free(SocketContext& p_ctx)
    {
        this->ask_end_thread = 1;
        this->server.free(p_ctx);
        Thread::wait_for_end_and_terminate(this->thread, -1);
    };

    inline void sync_wait_for_allocation()
    {
        while (!this->sync.allocated)
        {
        }
    };

  private:
    inline static int8 main(SocketSocketServerSingleClientThread* thiz)
    {
        thiz->server = SocketServerSingleClient::allocate(*thiz->input.ctx, thiz->input.port);
        thiz->sync.allocated = 1;
        while (1)
        {
            thiz->server.wait_for_client(*thiz->input.ctx);
            thiz->socket_connection_establishment_func->operator()(thiz);

            if (thiz->ask_end_thread)
            {
                break;
            }
        }

        return 0;
    };
};

struct SocketClient
{
    Socket client_socket;

    inline static SocketClient allocate(SocketContext& p_ctx, const int32 p_port)
    {
        return SocketClient{Socket::allocate_as_client(p_ctx, p_port)};
    };

    inline void free(SocketContext& p_ctx)
    {
        this->client_socket.free_gracefully(p_ctx);
    };
};

template <class SocketConnectionEstablishmentFunc> struct SocketClientThread
{
    struct Input
    {
        SocketContext* ctx;
        int32 port;
    } input;
    thread_t thread;

    struct Sync
    {
        volatile int8 allocated;
    } sync;

    struct Exec
    {
        SocketClientThread<SocketConnectionEstablishmentFunc>* thiz;
        inline int8 operator()() const
        {
            return SocketClientThread<SocketConnectionEstablishmentFunc>::main(thiz);
        };
    } exec;

    SocketClient client;
    const SocketConnectionEstablishmentFunc* socket_connection_establishment_func;

    inline void start(SocketContext* p_ctx, const int32 p_port, const SocketConnectionEstablishmentFunc* p_socket_connection_establishment_func)
    {
        this->exec = Exec{this};
        this->input = {p_ctx, p_port};
        this->sync.allocated = 0;
        this->socket_connection_establishment_func = p_socket_connection_establishment_func;
        this->thread = Thread::spawn_thread(this->exec);
    };

    inline void free(SocketContext& p_ctx)
    {
        this->client.free(p_ctx);
        Thread::wait_for_end_and_terminate(this->thread, -1);
    };

    inline void sync_wait_for_allocation()
    {
        while (!this->sync.allocated)
        {
        }
    };

  private:
    inline static int8 main(SocketClientThread* thiz)
    {
        thiz->client = SocketClient::allocate(*thiz->input.ctx, thiz->input.port);
        thiz->sync.allocated = 1;
        thiz->socket_connection_establishment_func->operator()(thiz);
        return 0;
    };
};

/*
    The SocketRequestResponseConnection establish a request linstening connection to a socket and can send it back data if desired.
 */
struct SocketRequestResponseConnection
{
    Span<int8> request_buffer;
    Span<int8> response_buffer;

    inline static SocketRequestResponseConnection allocate_default()
    {
        return SocketRequestResponseConnection{Span<int8>::allocate(512), Span<int8>::allocate(512)};
    };

    inline void free()
    {
        this->request_buffer.free();
        this->response_buffer.free();
    };

    enum class ListenSendResponseReturnCode : int8
    {
        NOTHING = 0,
        SEND_RESPONSE = 1,
        ABORT_LISTENER = 2,
        SEND_RESPONSE_AND_ABORT_LISTENER = SEND_RESPONSE | ABORT_LISTENER
    };

    template <class RequestCallbackFunc> inline void listen(SocketContext& p_ctx, Socket& p_listened_socket, const RequestCallbackFunc& p_request_callback_func)
    {

        while (1)
        {
            SocketReturnCode l_receive_return = socket_receive(p_listened_socket.native_socket, this->request_buffer.Memory, this->request_buffer.Capacity);

            if (l_receive_return == SocketReturnCode::GRACEFULLY_CLOSED)
            {
                socket_close(p_ctx, &p_listened_socket.native_socket);
                break;
            }

            uimax l_response_size = 0;
            ListenSendResponseReturnCode l_return_code = p_request_callback_func(this->request_buffer.slice, this->response_buffer.slice, &l_response_size);
            if ((int8)l_return_code & (int8)(ListenSendResponseReturnCode::SEND_RESPONSE))
            {
#if __DEBUG
                assert_true(l_response_size != 0);
#endif
                SocketReturnCode l_send_return_code = socket_send(p_listened_socket.native_socket, this->response_buffer.Memory, l_response_size);
                if (l_send_return_code == SocketReturnCode::GRACEFULLY_CLOSED)
                {
                    socket_close(p_ctx, &p_listened_socket.native_socket);
                    break;
                }
            }
            if (((int8)l_return_code & (int8)(ListenSendResponseReturnCode::ABORT_LISTENER)))
            {
                break;
            }
        }
    };
};

/*
    The SocketRequestConnection establish a request linstening connection to a socket.
 */
struct SocketRequestConnection
{
    Span<int8> request_buffer;

    inline static SocketRequestConnection allocate_default()
    {
        return SocketRequestConnection{Span<int8>::allocate(512)};
    };

    inline void free()
    {
        this->request_buffer.free();
    };

    enum class ListenSendResponseReturnCode : int8
    {
        NOTHING = 0,
        ABORT_LISTENER = 1
    };

    template <class RequestCallbackFunc> inline void listen(SocketContext& p_ctx, Socket& p_listened_socket, const RequestCallbackFunc& p_request_callback_func)
    {
        while (1)
        {
            SocketReturnCode l_receive_return = socket_receive(p_listened_socket.native_socket, this->request_buffer.Memory, this->request_buffer.Capacity);
            if (l_receive_return == SocketReturnCode::GRACEFULLY_CLOSED)
            {
                socket_close(p_ctx, &p_listened_socket.native_socket);
                break;
            }

            ListenSendResponseReturnCode l_return_code = p_request_callback_func(this->request_buffer.slice);
            if (((int8)l_return_code & (int8)(ListenSendResponseReturnCode::ABORT_LISTENER)))
            {
                break;
            }
        }
    };
};

struct SocketSendConnection
{
    Span<int8> send_buffer;

    inline static SocketSendConnection allocate_default()
    {
        return SocketSendConnection{Span<int8>::allocate(512)};
    };

    inline void free()
    {
        this->send_buffer.free();
    };

    inline SocketReturnCode send(Socket& p_listened_socket)
    {
        return p_listened_socket.send(this->send_buffer.slice);
    };
};

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

/*
    This is a functional object that helps to manipulate socket typed requests.
    Socket types request are a request code with a body.
 */
struct SocketTypedRequest
{
    int32* code;
    Slice<int8> payload;

    inline static SocketTypedRequest build(const Slice<int8>& p_request)
    {
        SocketTypedRequest l_return;
        BinaryDeserializer l_binary_desezrialiazer = BinaryDeserializer::build(p_request);
        l_return.code = l_binary_desezrialiazer.type<int32>();
        l_return.payload = l_binary_desezrialiazer.memory;
        return l_return;
    };

    template <class ElementType> inline void set_typed(const int32 p_code, const ElementType& p_element)
    {
        *this->code = p_code;
        BinarySerializer::slice(&this->payload, Slice<ElementType>::build_asint8_memory_singleelement(&p_element));
    };

    template <class ElementType> inline void get_typed(ElementType** out_element)
    {
        Slice<int8> l_element = BinaryDeserializer::build(this->payload).slice();
        *out_element = slice_cast<ElementType>(l_element).Begin;
    };
};

/*
    This is a functional object that helps to manipulate socket typed responses.
    Socket types response are a request code with a body.
 */
struct SocketTypedResponse
{
    int32* code;
    Slice<int8> payload;
    uimax payload_size;

    inline static SocketTypedResponse build(const Slice<int8>& p_request)
    {
        SocketTypedResponse l_return;
        BinaryDeserializer l_binary_desezrialiazer = BinaryDeserializer::build(p_request);
        l_return.code = l_binary_desezrialiazer.type<int32>();
        l_return.payload = l_binary_desezrialiazer.memory;
        l_return.payload_size = 0;
        return l_return;
    };

    inline uimax get_buffer_size()
    {
        return sizeof(*this->code) + this->payload_size;
    };

    inline void set(const int32 p_code, const Slice<int8>& p_element)
    {
        *this->code = p_code;
        this->payload_size += BinarySerializer::slice_ret_bytesnb(&this->payload, p_element);
    };

    inline void set_2(const int32 p_code, const Slice<int8>& p_element_0, const Slice<int8>& p_element_1)
    {
        *this->code = p_code;
        this->payload_size += BinarySerializer::slice_ret_bytesnb(&this->payload, p_element_0);
        this->payload_size += BinarySerializer::slice_ret_bytesnb(&this->payload, p_element_1);
    };

    template <class ElementType> inline void set_typed(const int32 p_code, const ElementType& p_element)
    {
        this->set(p_code, Slice<ElementType>::build_asint8_memory_singleelement(&p_element));
    };
};