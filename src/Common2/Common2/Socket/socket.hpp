#pragma once

#include <winsock2.h>
#include <ws2tcpip.h>

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
    // The os ::send can send less bytes tha the requested, so we iterate until all bytes have been sent
    int32 l_total_bytes_already_sent = 0;
    while (l_total_bytes_already_sent < p_size)
    {
        int32 l_bytes_sent = ::send(p_socket, p_begin + l_total_bytes_already_sent, (int)(p_size - l_total_bytes_already_sent), 0);

        if (l_bytes_sent == 0)
        {
            return SocketReturnCode::GRACEFULLY_SHUT;
        }
        else if (l_bytes_sent == SOCKET_ERROR)
        {
            return SocketReturnCode::ERR;
        }

        l_total_bytes_already_sent += l_bytes_sent;
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
#if __DEBUG
        assert_true(out_socket->state_is_free());
#endif

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
// TODO how to handle a buffer size that is lower than the messe received ?
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

            Slice<int8> l_response_slice = Slice<int8>{0, this->response_buffer.slice.Begin};
            ListenSendResponseReturnCode l_return_code = p_request_callback_func(this->request_buffer.slice, this->response_buffer.slice, &l_response_slice);
            if ((int8)l_return_code & (int8)(ListenSendResponseReturnCode::SEND_RESPONSE))
            {
#if __DEBUG
                assert_true(l_response_slice.Begin == this->response_buffer.slice.Begin);
                assert_true(this->response_buffer.slice.compare(l_response_slice));
#endif
                if (!p_listened_socket.send(p_ctx, l_response_slice))
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
// TODO how to handle a buffer size that is lower than the messe received ?
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

// TODO -> size can be dynamic
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

    inline int8 send_v2(SocketContext& p_ctx, Socket& p_listened_socket, const Slice<int8> p_send_buffer_slice)
    {
#if __DEBUG
        assert_true(this->send_buffer.Memory == p_send_buffer_slice.Begin);
        assert_true(this->send_buffer.slice.compare(p_send_buffer_slice));
#endif

        return p_listened_socket.send(p_ctx, p_send_buffer_slice);
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
        // The registered client must already be closed. Either gracefully when the listener has exited. Or by the server thread.
        // /!\ It is very important to not close the registerd_client_socket here because we can free the server without even calling the listen_request_response.

        this->server_socket.close(p_ctx);

#if __DEBUG
        assert_true(this->server_socket.state_is_free());
        assert_true(this->registerd_client_socket.state_is_free());
#endif
    };

    inline void free_registered_client_socket(SocketContext& p_ctx)
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
    };

    inline int8 wait_for_client(SocketContext& p_ctx)
    {
        return this->server_socket.accept_silent(&this->registerd_client_socket);
    };

    // called when the registerd_client_socket has been registered
    template <class RequestHandleFunc> inline void listen_request_response(SocketContext& p_ctx, const RequestHandleFunc& p_request_handle_func)
    {
        SocketRequestResponseConnection l_request_response_connection = SocketRequestResponseConnection::allocate_default();
        l_request_response_connection.listen(p_ctx, this->registerd_client_socket, p_request_handle_func);
        l_request_response_connection.free();
    };

    template <class RequestHandleFunc> inline void listen_request_response_v2(SocketContext& p_ctx, const RequestHandleFunc& p_request_handle_func)
    {
        while (1)
        {
            SocketRequestResponseConnection l_request_response_connection = SocketRequestResponseConnection::allocate_default();
            l_request_response_connection.listen(p_ctx, this->registerd_client_socket, p_request_handle_func);
            l_request_response_connection.free();
        }
    };
};

struct SocketClient
{
    Socket client_socket;
    SocketSendConnection client_send_connection;

    inline static SocketClient allocate(SocketContext& p_ctx, const int32 p_port)
    {
        return SocketClient{Socket::allocate_as_client(p_ctx, p_port), SocketSendConnection::allocate_default()};
    };

    inline void free(SocketContext& p_ctx)
    {
        this->client_socket.free_gracefully(p_ctx);
        this->client_socket.close(p_ctx);
        this->client_send_connection.free();
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

    template <class RequestHandleFunc> inline void listen_request(SocketContext& p_ctx, const RequestHandleFunc& p_request_handle_func)
    {
        SocketRequestConnection l_request_connection = SocketRequestConnection::allocate_default();
        l_request_connection.listen(p_ctx, this->client_socket, p_request_handle_func);
        l_request_connection.free();
    };

    inline const Slice<int8>& get_client_to_server_buffer()
    {
        return this->client_send_connection.send_buffer.slice;
    };

    inline void send_client_to_server(SocketContext& p_ctx, const Slice<int8>& p_sended_slice)
    {
        this->client_send_connection.send_v2(p_ctx, this->client_socket, p_sended_slice);
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
        this->server.free(p_ctx);
        this->ask_end_thread = 1;
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
            thiz->server.free_registered_client_socket(*thiz->input.ctx);

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

// TODO update to allow resize of socket connection buffer
struct SocketRequestWriter
{
    inline static Slice<int8> set(const Slice<int8>& p_element, const Slice<int8>& p_write_to_buffer)
    {
        Slice<int8> l_target = p_write_to_buffer;
        l_target.Size = p_element.Size;
        l_target.copy_memory(p_element);
        return l_target;
    };

    inline static Slice<int8> set_2(const Slice<int8>& p_element_1, const Slice<int8>& p_element_2, const Slice<int8>& p_write_to_buffer)
    {
        Slice<int8> l_target = p_write_to_buffer;
        l_target.Size = p_element_1.Size + p_element_2.Size;
        l_target.copy_memory_2(p_element_1, p_element_2);
        return l_target;
    };

    inline static Slice<int8> set_3(const Slice<int8>& p_element_1, const Slice<int8>& p_element_2, const Slice<int8>& p_element_3, const Slice<int8>& p_write_to_buffer)
    {
        Slice<int8> l_target = p_write_to_buffer;
        l_target.Size = p_element_1.Size + p_element_2.Size + p_element_3.Size;
        l_target.copy_memory_3(p_element_1, p_element_2, p_element_3);
        return l_target;
    };
};

/*
    This is a functional object that helps to manipulate socket typed requests.
    Socket types request are a request code with a body.
 */
struct SocketTypedRequestReader
{
    int32* code;
    Slice<int8> payload;

    inline static SocketTypedRequestReader build(const Slice<int8>& p_request)
    {
        SocketTypedRequestReader l_return;
        BinaryDeserializer l_binary_desezrialiazer = BinaryDeserializer::build(p_request);
        l_return.code = l_binary_desezrialiazer.type<int32>();
        l_return.payload = l_binary_desezrialiazer.memory;
        return l_return;
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
// TODO update to allow resize of socket connection buffer
struct SocketTypedRequestWriter
{

    inline static Slice<int8> set(const int32 p_code, const Slice<int8>& p_element, const Slice<int8>& p_write_to_buffer)
    {
        VectorSlice<int8> l_target = VectorSlice<int8>::build(p_write_to_buffer, 0);
        Slice<int8> l_return = p_write_to_buffer;
        l_return.Size = 0;
        BinarySerializer::type(l_target.to_ivector(), p_code);
        l_return.Size += sizeof(p_code);
        l_return.Size += BinarySerializer::slice_ret_bytesnb(l_target.to_ivector(), p_element);
        return l_return;
    };
};

struct SocketTypedHeaderRequestReader
{
    int32* code;
    Slice<int8> header;
    Slice<int8> body;

    inline static SocketTypedHeaderRequestReader build(const Slice<int8>& p_request)
    {
        SocketTypedHeaderRequestReader l_return;
        BinaryDeserializer l_binary_desezrialiazer = BinaryDeserializer::build(p_request);
        l_return.code = l_binary_desezrialiazer.type<int32>();
        l_return.header = l_binary_desezrialiazer.slice();
        l_return.body = l_binary_desezrialiazer.slice();
        return l_return;
    };
};

// TODO update to allow resize of socket connection buffer
struct SocketTypedHeaderRequestWriter
{
    inline static Slice<int8> set(const int32 p_code, const Slice<int8>& p_header, const Slice<int8>& p_body, const Slice<int8>& p_writeto_buffer)
    {
        VectorSlice<int8> l_target = VectorSlice<int8>::build(p_writeto_buffer, 0);
        Slice<int8> l_return = p_writeto_buffer;
        l_return.Size = 0;
        BinarySerializer::type(l_target.to_ivector(), p_code);
        l_return.Size += sizeof(p_code);
        l_return.Size += BinarySerializer::slice_ret_bytesnb(l_target.to_ivector(), p_header);
        l_return.Size += BinarySerializer::slice_ret_bytesnb(l_target.to_ivector(), p_body);
        return l_return;
    }

    inline static Slice<int8> set_code_header(const int32 p_code, const Slice<int8>& p_header, const Slice<int8>& p_writeto_buffer)
    {
        VectorSlice<int8> l_target = VectorSlice<int8>::build(p_writeto_buffer, 0);
        Slice<int8> l_return = p_writeto_buffer;
        l_return.Size = 0;
        BinarySerializer::type(l_target.to_ivector(), p_code);
        l_return.Size += sizeof(p_code);
        l_return.Size += BinarySerializer::slice_ret_bytesnb(l_target.to_ivector(), p_header);
        return l_return;
    }
};

/* Structure that allows syncrhonous execution of client->server->client request and response. */
struct SocketRequestResponseTracker
{
    struct Response
    {
        // This boolean is updatedby the client thread to notify consumer that the server request response has been received. It must be volatile because it accessed by the client trhead and the main
        // thread.
        volatile int8 is_processed;
        // The buffer that will be updated by the client one server response has been received
        Slice<int8> client_response_slice;

        inline static Response build(const Slice<int8>& p_client_return_slice)
        {
            return Response{0, p_client_return_slice};
        };
    };

    Pool<Response> response_closures;

    inline static SocketRequestResponseTracker allocate()
    {
        return SocketRequestResponseTracker{Pool<Response>::allocate(0)};
    };

    inline void free()
    {
#if __DEBUG
        assert_true(!this->response_closures.has_allocated_elements());
#endif
        this->response_closures.free();
    };

    inline void wait_for_request_completion(const Token<Response> p_request)
    {
        int8 l_is_processed = 0;
        Response& l_closure = this->response_closures.get(p_request);
        while (!l_is_processed)
        {
            // TODO -> having a timer ?
            l_is_processed = l_closure.is_processed;
        }
        this->response_closures.release_element(p_request);
    };

    inline void set_response_slice(const Token<Response> p_request, const Slice<int8>& p_response_slice)
    {
        SocketRequestResponseTracker::Response& l_closure = this->response_closures.get(p_request);
        l_closure.client_response_slice.copy_memory(p_response_slice);
    };

    inline void set_request_is_completed(const Token<Response> p_request)
    {
        this->response_closures.get(p_request).is_processed = 1;
    };
};
