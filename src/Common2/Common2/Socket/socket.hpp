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

/*
    Shutdwon the socket gracefully by consuming remaining events.
    /!\ It it up to the owning structure to manually call the socket_close to effectively close the socket.
 */
inline void socket_graceful_shut(SocketContext& p_ctx, socket_t* p_socket)
{
#if __DEBUG
    assert_true(*p_socket != INVALID_SOCKET);
#endif
    HANDLE l_wsa_event = WSACreateEvent();
    WSAEventSelect(*p_socket, l_wsa_event, FD_CLOSE);
    shutdown(*p_socket, SD_BOTH);
    WSAWaitForMultipleEvents(1, &l_wsa_event, TRUE, WSA_INFINITE, TRUE);
};

enum class SocketReturnCode
{
    IDLING = 0,
    GRACEFULLY_SHUT = 1,
    ERR = 2
};

inline SocketReturnCode socket_receive(socket_t p_socket, int8* p_begin, const uimax p_size)
{
    int l_result = ::recv(p_socket, p_begin, (int)p_size, 0);

    if (l_result == 0)
    {
        return SocketReturnCode::GRACEFULLY_SHUT;
    }
    else if (l_result == SOCKET_ERROR)
    {
        return SocketReturnCode::ERR;
    }
    return SocketReturnCode::IDLING;
};

inline SocketReturnCode socket_send(socket_t p_socket, const int8* p_begin, const uimax p_size)
{
    int l_result = ::send(p_socket, p_begin, (int)(p_size), 0);

    if (l_result == 0)
    {
        return SocketReturnCode::GRACEFULLY_SHUT;
    }
    else if (l_result == SOCKET_ERROR)
    {
        return SocketReturnCode::ERR;
    }
    return SocketReturnCode::IDLING;
};

inline int8 socket_accept_silent(socket_t p_socket, socket_t* out_socket)
{
    *out_socket = ::accept(p_socket, NULL, NULL);
#if __MEMLEAK
    if (*out_socket != INVALID_SOCKET)
    {
        push_ptr_to_tracked((int8*)*out_socket);
    }
#endif
    return *out_socket != INVALID_SOCKET;
};

/*
    Structure that manages allocation/release of a socket.
    When the socket is gracefully freed,
 */
struct Socket
{
    enum class State : int8
    {
        UNDEFINED = 0,
        ALLOCATED = 1,
        /*
            The socket has been gracefully shutted, it still need to be freed. The gracefully shut just means that the remaining events have been consumed.
        */
        GRACEFULLY_SHUT = 2,
        FREE = 3
    };

    // Sockets are usually instanciated in thread. Socket threads make heavily use of the socket state for sycnhronizing or check if the socket is being gracefully_shut. So we set this value as
    // volatile.
    volatile State state;

    socket_t native_socket;
    addrinfo* native_addr_info;

    inline static Socket build_default()
    {
        return Socket{State::UNDEFINED, INVALID_SOCKET, NULL};
    };

    inline static Socket allocate_as_server(SocketContext& p_ctx, const int32 p_port)
    {
        addrinfo l_addr_info{};
        l_addr_info.ai_family = AF_INET;
        l_addr_info.ai_socktype = SOCK_STREAM;
        l_addr_info.ai_protocol = IPPROTO_TCP;
        l_addr_info.ai_flags = AI_PASSIVE;

        Socket l_socket;
        _allocate_socket_and_bind_and_listen(l_socket, p_ctx, l_addr_info, p_port);
        l_socket.state = State::ALLOCATED;
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
        l_socket.state = State::ALLOCATED;
        return l_socket;
    };

    inline void free_gracefully(SocketContext& p_ctx)
    {
        // /!\ it is very important to set the socket state before calling native functions because the effective deallocation of the socket is deferred until all requests are completed
        this->state = State::GRACEFULLY_SHUT;
        freeaddrinfo(this->native_addr_info);
        this->native_addr_info = NULL;
        socket_graceful_shut(p_ctx, &this->native_socket);
    };

    inline void close(SocketContext& p_ctx)
    {
        // /!\ it is very important to set the socket state before calling native functions because the effective deallocation of the socket is deferred until all requests are completed
        this->state = State::FREE;
        freeaddrinfo(this->native_addr_info);
        this->native_addr_info = NULL;
        socket_close(p_ctx, &this->native_socket);
    };

    inline int8 state_is_allocated()
    {
        return this->state == State::ALLOCATED;
    };

    inline int8 state_if_free_gracefully()
    {
        return this->state == State::GRACEFULLY_SHUT;
    };

    inline int8 state_is_free()
    {
        return this->state == State::FREE || this->state == State::UNDEFINED;
    };

    inline int8 receive(SocketContext& p_ctx, Slice<int8>& p_buffer)
    {
        SocketReturnCode l_receive_return = socket_receive(this->native_socket, p_buffer.Begin, p_buffer.Size);

        if (l_receive_return == SocketReturnCode::GRACEFULLY_SHUT)
        {
            if (this->state_is_allocated())
            {
                this->free_gracefully(p_ctx);
            }
            return 0;
        }
        else if (l_receive_return == SocketReturnCode::ERR)
        {
            if (this->state_is_allocated())
            {
                // We simulate the fract that the socket has been gracefully shut to avoid infinite loop while waiting if the graceful_shut was waiting for remaining events
                this->state = Socket::State::GRACEFULLY_SHUT;
            }
            return 0;
        }
        return 1;
    };

    inline int8 send(SocketContext& p_ctx, const Slice<int8>& p_buffer)
    {
        SocketReturnCode l_send_return_code = socket_send(this->native_socket, p_buffer.Begin, p_buffer.Size);
        if (l_send_return_code == SocketReturnCode::GRACEFULLY_SHUT)
        {
            if (this->state_is_allocated())
            {
                this->free_gracefully(p_ctx);
            }
            return 0;
        }
        else if (l_send_return_code == SocketReturnCode::ERR)
        {
            if (this->state_is_allocated())
            {
                this->state = Socket::State::GRACEFULLY_SHUT;
            }
            return 0;
        }
        return 1;
    }

    inline int8 accept_silent(Socket* out_socket)
    {
        int8 l_return = socket_accept_silent(this->native_socket, &out_socket->native_socket);
        if (l_return)
        {
            out_socket->state = State::ALLOCATED;
        }
        else
        {
            out_socket->state = State::UNDEFINED;
        }
        return l_return;
    };

  private:
    inline static void _allocate_socket_and_bind_and_listen(Socket& p_socket, SocketContext& p_ctx, const addrinfo& p_addr_info, const int32 p_port)
    {
        int8 p_port_str_raw[ToString::int32str_size];
        ToString::aint32(p_port, slice_int8_build_rawstr(p_port_str_raw));

        p_socket.native_addr_info = NULL;
        winsock_error_handler(getaddrinfo(NULL, p_port_str_raw, &p_addr_info, &p_socket.native_addr_info));

        p_socket.native_socket = socket_allocate(p_ctx, socket(p_socket.native_addr_info->ai_family, p_socket.native_addr_info->ai_socktype, p_socket.native_addr_info->ai_protocol));
        winsock_error_handler(bind(p_socket.native_socket, p_socket.native_addr_info->ai_addr, (int)p_socket.native_addr_info->ai_addrlen));

        winsock_error_handler(listen(p_socket.native_socket, SOMAXCONN));
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
                p_socket.close(p_ctx);
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
        SEND_RESPONSE = 1
    };

    template <class RequestCallbackFunc> inline void listen(SocketContext& p_ctx, Socket& p_listened_socket, const RequestCallbackFunc& p_request_callback_func)
    {

        while (1)
        {
            if (!p_listened_socket.receive(p_ctx, this->request_buffer.slice))
            {
                break;
            }

            uimax l_response_size = 0;
            ListenSendResponseReturnCode l_return_code = p_request_callback_func(this->request_buffer.slice, this->response_buffer.slice, &l_response_size);
            if ((int8)l_return_code & (int8)(ListenSendResponseReturnCode::SEND_RESPONSE))
            {
#if __DEBUG
                assert_true(l_response_size != 0);
#endif
                if (!p_listened_socket.send(p_ctx, Slice<int8>::build_memory_elementnb(this->response_buffer.Memory, l_response_size)))
                {
                    break;
                };
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

    template <class RequestCallbackFunc> inline void listen(SocketContext& p_ctx, Socket& p_listened_socket, const RequestCallbackFunc& p_request_callback_func)
    {
        while (1)
        {
            if (!p_listened_socket.receive(p_ctx, this->request_buffer.slice))
            {
                break;
            }

            p_request_callback_func(this->request_buffer.slice);
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

    inline int8 send(SocketContext& p_ctx, Socket& p_listened_socket)
    {
        return p_listened_socket.send(p_ctx, this->send_buffer.slice);
    };
};

struct SocketServerSingleClient
{
    Socket server_socket;
    Socket registerd_client_socket;

    inline static SocketServerSingleClient allocate(SocketContext& p_ctx, const int32 p_port)
    {
        return SocketServerSingleClient{Socket::allocate_as_server(p_ctx, p_port), Socket::build_default()};
    };

    inline void free(SocketContext& p_ctx)
    {
        // some client may already be allocated or not. We wave no way to know it at compile time because the registerd_client_socket may be deallocated when the client disconnect.
        if (this->registerd_client_socket.state_is_allocated())
        {
            this->registerd_client_socket.free_gracefully(p_ctx);
            this->registerd_client_socket.close(p_ctx);
        }
        else if (this->registerd_client_socket.state_if_free_gracefully())
        {
            this->registerd_client_socket.close(p_ctx);
        }
        this->server_socket.close(p_ctx);

#if __DEBUG
        assert_true(this->server_socket.state_is_free());
        assert_true(this->registerd_client_socket.state_is_free());
#endif
    };

    inline int8 wait_for_client(SocketContext& p_ctx)
    {
        return this->server_socket.accept_silent(&this->registerd_client_socket);
    };

    template <class RequestHandleFunc> inline void listen_request_response(SocketContext& p_ctx, const RequestHandleFunc& p_request_handle_func)
    {
        SocketRequestResponseConnection l_request_response_connection = SocketRequestResponseConnection::allocate_default();
        l_request_response_connection.listen(p_ctx, this->registerd_client_socket, p_request_handle_func);
        l_request_response_connection.free();
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
        this->client_socket.close(p_ctx);
#if __DEBUG
        assert_true(this->client_socket.state_is_free());
#endif
    };

    template <class RequestHandleFunc> inline void listen_request_response(SocketContext& p_ctx, const RequestHandleFunc& p_request_handle_func)
    {
        SocketRequestResponseConnection l_request_response_connection = SocketRequestResponseConnection::allocate_default();
        l_request_response_connection.listen(p_ctx, this->client_socket, p_request_handle_func);
        l_request_response_connection.free();
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
        this->server = SocketServerSingleClient::allocate(*p_ctx, p_port);
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
        while (!this->server.server_socket.state_is_allocated())
        {
        }
    };

    inline void sync_wait_for_client_detection()
    {
        while (!this->server.registerd_client_socket.state_is_allocated())
        {
        }
    };

  private:
    inline static int8 main(SocketSocketServerSingleClientThread<SocketConncetionEstablishmentFunc>* thiz)
    {
        while (1)
        {
            if (!thiz->server.wait_for_client(*thiz->input.ctx))
            {
                break;
            }

            thiz->socket_connection_establishment_func->operator()();

            if (thiz->ask_end_thread)
            {
                break;
            }
        }

        return 0;
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
        while (!this->client.client_socket.state_is_allocated())
        {
        }
    };

  private:
    inline static int8 main(SocketClientThread* thiz)
    {
        thiz->client = SocketClient::allocate(*thiz->input.ctx, thiz->input.port);
        thiz->socket_connection_establishment_func->operator()();
        return 0;
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