#pragma once

#define SOCKET_DEFAULT_PORT 8000

#if _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")
#pragma comment(lib, "AdvApi32.lib")
#endif

using socket_t =
#if _WIN32
    SOCKET;
#else
    int;
#endif

struct SocketNative
{
    template <class ReturnType> inline static ReturnType error_handler(const ReturnType p_return)
    {
#if __DEBUG
        if (p_return != 0)
        {
            abort();
        }
#endif
        return p_return;
    };

    inline static socket_t socket_allocate_bind(addrinfo* p_native_addr_info)
    {
        socket_t l_socket = socket(p_native_addr_info->ai_family, p_native_addr_info->ai_socktype, p_native_addr_info->ai_protocol);
#if __DEBUG
        assert_true(l_socket != INVALID_SOCKET);
#endif
        SocketNative::error_handler(bind(l_socket, p_native_addr_info->ai_addr, (int)p_native_addr_info->ai_addrlen));
        return l_socket;
    };

    inline static socket_t socket_allocate_connect(addrinfo* p_native_addr_info)
    {
        socket_t l_socket;

        addrinfo* l_ptr;
        addrinfo* l_result = p_native_addr_info;
        for (l_ptr = l_result; l_ptr != NULL; l_ptr = l_ptr->ai_next)
        {
            l_socket = socket(l_result->ai_family, l_result->ai_socktype, l_result->ai_protocol);
            if (connect(l_socket, l_result->ai_addr, (int)l_result->ai_addrlen) != 0)
            {
                socket_close(&l_socket);
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

        return l_socket;
    };

    inline static void socket_connect(const socket_t& p_socket, const addrinfo* p_addr_info)
    {
        SocketNative::error_handler(connect(p_socket, p_addr_info->ai_addr, (int)p_addr_info->ai_addrlen));
    };

    inline static void socket_close(socket_t* p_socket)
    {
#if __DEBUG
        assert_true(*p_socket != INVALID_SOCKET);
#endif
        closesocket(*p_socket);
        *p_socket = INVALID_SOCKET;
    };

    /*
    Shutdwon the socket gracefully by consuming remaining events.
    /!\ It it up to the owning structure to manually call the socket_close to effectively close the socket.
 */
    inline static void socket_graceful_shut(socket_t* p_socket)
    {
#if __DEBUG
        assert_true(*p_socket != INVALID_SOCKET);
#endif
#if _WIN32
        HANDLE l_wsa_event = WSACreateEvent();
        WSAEventSelect(*p_socket, l_wsa_event, FD_CLOSE);
        shutdown(*p_socket, SD_BOTH);
        WSAWaitForMultipleEvents(1, &l_wsa_event, TRUE, WSA_INFINITE, TRUE);
#else
        SocketNative::error_handler(shutdown(*p_socket, SHUT_RDWR));
#endif
    };
};

struct SocketContext
{
    inline static void allocate()
    {
        context_startup();
    };

    inline static void free()
    {
        context_cleanup();
    };

  private:
    inline static void context_startup()
    {
        WSADATA l_winsock_data;
        SocketNative::error_handler(WSAStartup(MAKEWORD(2, 2), &l_winsock_data));
    };
    inline static void context_cleanup()
    {
        WSACleanup();
    };
};

struct SocketServerNonBlocking
{
#if __MEMLEAK
    int8* memleak_ptr;
#endif
    socket_t native_socket;
    addrinfo* native_addr_info;

    struct AcceptState
    {
        int8 enabled;
    } accept_state;

    struct AcceptReturn
    {
        socket_t acceped_socket;
    } accept_return;

    inline static SocketServerNonBlocking allocate(const int32 p_port)
    {
        addrinfo l_addr_info{};
        l_addr_info.ai_family = AF_INET;
        l_addr_info.ai_socktype = SOCK_STREAM;
        l_addr_info.ai_protocol = IPPROTO_TCP;
        l_addr_info.ai_flags = AI_PASSIVE;

        SocketServerNonBlocking l_socket;
        _allocate_socket_and_bind_and_listen(l_socket, l_addr_info, p_port);

        _initialize_states(l_socket);

#if __MEMLEAK
        l_socket.memleak_ptr = heap_malloc(sizeof(*l_socket.memleak_ptr));
        push_ptr_to_tracked(l_socket.memleak_ptr);
#endif

        return l_socket;
    };

    inline void close()
    {
        // /!\ it is very important to set the socket state before calling native functions because the effective deallocation of the socket is deferred until all requests are completed
        freeaddrinfo(this->native_addr_info);
        this->native_addr_info = NULL;
        SocketNative::socket_close(&this->native_socket);
#if __MEMLEAK
        remove_ptr_to_tracked(this->memleak_ptr);
        heap_free(this->memleak_ptr);
#endif
    };

    inline void accept()
    {
        this->accept_state.enabled = 1;
        this->accept_return.acceped_socket = INVALID_SOCKET;
    };

    inline void step()
    {
        if (this->accept_state.enabled)
        {
            this->accept_return.acceped_socket = ::accept(this->native_socket, NULL, NULL);
            if (this->accept_return.acceped_socket == INVALID_SOCKET)
            {
                if (WSAGetLastError() != WSAEWOULDBLOCK)
                {
                    this->accept_state.enabled = 0;
                }
            }
        }
    };

  private:
    inline static void _allocate_socket_and_bind_and_listen(SocketServerNonBlocking& p_socket, const addrinfo& p_addr_info, const int32 p_port)
    {
        SliceN<int8, ToString::int32str_size> p_port_str_raw;
        Slice<int8> l_port_str = slice_from_slicen(&p_port_str_raw);
        ToString::aint32(p_port, l_port_str);

        p_socket.native_addr_info = NULL;
        SocketNative::error_handler(getaddrinfo(NULL, l_port_str.Begin, &p_addr_info, &p_socket.native_addr_info));

        p_socket.native_socket = SocketNative::socket_allocate_bind(p_socket.native_addr_info);
        uint32 l_mode = 1; // non-blocking
        SocketNative::error_handler(ioctlsocket(p_socket.native_socket, FIONBIO, (u_long*)&l_mode));

        SocketNative::error_handler(listen(p_socket.native_socket, SOMAXCONN));
    };

    inline static void _initialize_states(SocketServerNonBlocking& p_socket)
    {
        p_socket.accept_state.enabled = 0;
        p_socket.accept_return.acceped_socket = INVALID_SOCKET;
    };
};

struct SocketClientNonBlocking
{
#if __MEMLEAK
    int8* memleak_ptr;
#endif
    socket_t native_socket;
    addrinfo* native_addr_info;

    struct SendState
    {
        int8 enabled;
        uimax sended_bytes;
    } send_state;

    struct SendInput
    {
        Slice<int8> sended_buffer;
    } send_input;

    struct ReceiveState
    {
        int8 enabled;
        uimax received_bytes_total;
        uimax received_bytes;
    } receive_state;

    struct ReceiveInput
    {
        Slice<int8> target_buffer;
    } receive_input;

    inline static SocketClientNonBlocking allocate(const int32 p_port)
    {
        addrinfo l_addr_info{};
        l_addr_info.ai_family = AF_INET;
        l_addr_info.ai_socktype = SOCK_STREAM;
        l_addr_info.ai_protocol = IPPROTO_TCP;

        SocketClientNonBlocking l_socket;
        _allocate_socket_and_connect(l_socket, l_addr_info, p_port);

        _initialize_states(l_socket);

#if __MEMLEAK
        l_socket.memleak_ptr = heap_malloc(sizeof(*l_socket.memleak_ptr));
        push_ptr_to_tracked(l_socket.memleak_ptr);
#endif

        return l_socket;
    };

    inline static SocketClientNonBlocking allocate_from_native(const socket_t p_native_socket)
    {
        SocketClientNonBlocking l_socket;
        l_socket.native_socket = p_native_socket;
        l_socket.native_addr_info = NULL;

        _initialize_states(l_socket);

#if __MEMLEAK
        l_socket.memleak_ptr = heap_malloc(sizeof(*l_socket.memleak_ptr));
        push_ptr_to_tracked(l_socket.memleak_ptr);
#endif

        return l_socket;
    };

    inline void close()
    {
        // /!\ it is very important to set the socket state before calling native functions because the effective deallocation of the socket is deferred until all requests are completed
        freeaddrinfo(this->native_addr_info);
        this->native_addr_info = NULL;
        SocketNative::socket_close(&this->native_socket);
#if __MEMLEAK
        remove_ptr_to_tracked(this->memleak_ptr);
        heap_free(this->memleak_ptr);
#endif
    };

    inline void send(const SendInput& p_send_input)
    {
        this->send_state.sended_bytes = 0;
        this->send_input = p_send_input;
        this->send_state.enabled = 1;
    };

    inline void receive(const ReceiveInput& p_receive_input)
    {
        this->receive_state.received_bytes = 0;
        this->receive_state.received_bytes_total = 0;
        this->receive_input = p_receive_input;
        this->receive_state.enabled = 1;
    };

    inline void step()
    {
        if (this->send_state.enabled)
        {
            if (this->send_state.sended_bytes < this->send_input.sended_buffer.Size)
            {
                int32 l_bytes_sent =
                    ::send(this->native_socket, this->send_input.sended_buffer.Begin + this->send_state.sended_bytes, (int)(this->send_input.sended_buffer.Size - this->send_state.sended_bytes), 0);

                if (l_bytes_sent == INVALID_SOCKET)
                {
                    int32 l_last_error = WSAGetLastError();
                    if (l_last_error != WSAEWOULDBLOCK)
                    {
                        this->send_state.enabled = 0;
                    }
                }
                else
                {
                    if (l_bytes_sent == 0 || l_bytes_sent == SOCKET_ERROR)
                    {
                        this->send_state.enabled = 0;
                    }
                    else
                    {
                        this->send_state.sended_bytes += l_bytes_sent;
                        if (this->send_state.sended_bytes == this->send_input.sended_buffer.Size)
                        {
                            this->send_state.enabled = 0;
                        }
                    }
                }
            }
            else
            {
                this->send_state.enabled = 0;
            }
        }

        if (this->receive_state.enabled)
        {
            int32 l_bytes_received = ::recv(this->native_socket, this->receive_input.target_buffer.Begin, (int32)this->receive_input.target_buffer.Size, 0);

            if (l_bytes_received == SOCKET_ERROR)
            {
                int32 l_last_error = WSAGetLastError();
                if (l_last_error != WSAEWOULDBLOCK)
                {
                    // There is still data to receive
                    if (l_last_error != WSAEMSGSIZE)
                    {
                        this->receive_state.enabled = 0;
                    }
                }
            }
            else if (l_bytes_received == 0)
            {
                this->receive_state.enabled = 0;
            }
            else
            {
                this->receive_state.received_bytes = l_bytes_received;
                this->receive_state.received_bytes_total += l_bytes_received;
                // thiz.receive_state.enabled = 0;
            }
        }
    };

  private:
    inline static void _allocate_socket_and_connect(SocketClientNonBlocking& p_socket, const addrinfo& p_addr_info, const int32 p_port)
    {
        _load_addrinfos(p_socket, p_addr_info, p_port);

        p_socket.native_socket = SocketNative::socket_allocate_connect(p_socket.native_addr_info);

        uint32 l_mode = 1; // non-blocking
        SocketNative::error_handler(ioctlsocket(p_socket.native_socket, FIONBIO, (u_long*)&l_mode));
    };

    inline static void _load_addrinfos(SocketClientNonBlocking& p_socket, const addrinfo& p_addr_info, const int32 p_port)
    {
        SliceN<int8, ToString::int32str_size> p_port_str_raw;
        Slice<int8> l_port_str = slice_from_slicen(&p_port_str_raw);
        ToString::aint32(p_port, l_port_str);

        p_socket.native_addr_info = NULL;
        SocketNative::error_handler(getaddrinfo(NULL, l_port_str.Begin, &p_addr_info, &p_socket.native_addr_info));
    };

    inline static void _initialize_states(SocketClientNonBlocking& p_socket)
    {
        p_socket.send_state.enabled = 0;

        p_socket.receive_state.enabled = 0;
    };
};
