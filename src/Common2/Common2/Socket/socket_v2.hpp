#pragma once

#define SOCKET_DEFAULT_PORT 8000

#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")
#pragma comment(lib, "AdvApi32.lib")

struct SocketNative
{
    inline static int32 get_last_error()
    {
#if _WIN32
        return WSAGetLastError();
#else
        return errno;
#endif
    };

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

    // If the socket returns an error that tells that the call should have blocked
    inline static int8 is_error_is_blocking_type(const int32 p_error)
    {
#if _WIN32
        return p_error == WSAEWOULDBLOCK;
#else
        return (p_error == EWOULDBLOCK || p_error == EAGAIN);
#endif
    };

    // If the socket returns an error that tells that there is still data to receive
    inline static int8 is_error_is_msgsize_type(const int32 p_error)
    {

#if _WIN32
        return p_error == WSAEMSGSIZE;
#else
        // for linux, such error doesn't exists, so we always return 0 indicating that it is not a msgsize type error.
        return 0;
#endif
    };

    inline static socket_native socket_allocate_bind(socket_addrinfo p_native_addr_info)
    {
        socket_native l_socket = socket_native_allocate(p_native_addr_info);
        socket_native_set_address_reusable(l_socket);
        socket_native_bind(l_socket, p_native_addr_info);
        return l_socket;
    };

    inline static socket_native socket_allocate_connect(socket_addrinfo p_native_addr_info)
    {
        socket_native l_socket;

        socket_addrinfo l_addr_info = p_native_addr_info;
        while (!l_addr_info.is_null())
        {
            l_socket = socket_native_allocate(l_addr_info);
            if (socket_native_connect(l_socket, l_addr_info))
            {
                socket_close(&l_socket);
                continue;
            }
            else
            {
                break;
            }

            l_addr_info = socket_addrinfo_next(l_addr_info);
        }

        if (l_addr_info.is_null())
        {
            ::abort();
        }

        socket_native_set_address_reusable(l_socket);

        return l_socket;
    };

    inline static void socket_close(socket_native* p_socket)
    {
#if __DEBUG
        p_socket->is_valid();
#endif
        socket_native_close(*p_socket);
        p_socket->invalidate();
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
#if _WIN32
        WSADATA l_winsock_data;
        SocketNative::error_handler(WSAStartup(MAKEWORD(2, 2), &l_winsock_data));
#endif
    };
    inline static void context_cleanup()
    {
#if _WIN32
        WSACleanup();
#endif
    };
};

struct SocketServerNonBlocking
{
#if __MEMLEAK
    int8* memleak_ptr;
#endif
    socket_native native_socket;
    socket_addrinfo native_addr_info;

    struct AcceptState
    {
        int8 enabled;
    } accept_state;

    struct AcceptReturn
    {
        socket_native acceped_socket;
    } accept_return;

    inline static SocketServerNonBlocking allocate(const int32 p_port)
    {
        SocketServerNonBlocking l_socket;
        _allocate_socket_and_bind_and_listen(l_socket, p_port);

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
        freeaddrinfo((addrinfo*)this->native_addr_info.ptr);
        this->native_addr_info.ptr = NULL;
        SocketNative::socket_close(&this->native_socket);
#if __MEMLEAK
        remove_ptr_to_tracked(this->memleak_ptr);
        heap_free(this->memleak_ptr);
#endif
    };

    inline void accept()
    {
        this->accept_state.enabled = 1;
        this->accept_return.acceped_socket.invalidate();
    };

    inline void step()
    {
        if (this->accept_state.enabled)
        {
            this->accept_return.acceped_socket.ptr = (int8*)::accept((SOCKET)this->native_socket.ptr, NULL, NULL);
            if (this->accept_return.acceped_socket.is_valid())
            {
                if (!SocketNative::is_error_is_blocking_type(SocketNative::get_last_error()))
                {
                    this->accept_state.enabled = 0;
                }
            }
        }
    };

  private:
    inline static void _allocate_socket_and_bind_and_listen(SocketServerNonBlocking& p_socket, const int32 p_port)
    {
        p_socket.native_addr_info = socket_addrinfo_get(socket_native_type::TCP, p_port);

        p_socket.native_socket = SocketNative::socket_allocate_bind(p_socket.native_addr_info);
        socket_native_set_non_blocking(p_socket.native_socket);

        SocketNative::error_handler(listen((SOCKET)p_socket.native_socket.ptr, SOMAXCONN));
    };

    inline static void _initialize_states(SocketServerNonBlocking& p_socket)
    {
        p_socket.accept_state.enabled = 0;
        p_socket.accept_return.acceped_socket.invalidate();
    };
};

struct SocketClientNonBlocking
{
#if __MEMLEAK
    int8* memleak_ptr;
#endif
    socket_native native_socket;
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

    inline static SocketClientNonBlocking allocate_from_native(const socket_native p_native_socket)
    {
        socket_native_set_non_blocking(p_native_socket);

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
                int32 l_bytes_sent = ::send((SOCKET)this->native_socket.ptr, this->send_input.sended_buffer.Begin + this->send_state.sended_bytes,
                                            (int)(this->send_input.sended_buffer.Size - this->send_state.sended_bytes), 0);

                if (l_bytes_sent == INVALID_SOCKET)
                {
                    if (!SocketNative::is_error_is_blocking_type(SocketNative::get_last_error()))
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
            int32 l_bytes_received = ::recv((SOCKET)this->native_socket.ptr, this->receive_input.target_buffer.Begin, (int32)this->receive_input.target_buffer.Size, 0);

            if (l_bytes_received == SOCKET_ERROR)
            {
                int32 l_error = SocketNative::get_last_error();
                if (!SocketNative::is_error_is_blocking_type(l_error))
                {
                    // There is still data to receive
                    if (!SocketNative::is_error_is_msgsize_type(l_error))
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

        p_socket.native_socket = SocketNative::socket_allocate_connect(socket_addrinfo{(int8*)p_socket.native_addr_info});
        socket_native_set_non_blocking(p_socket.native_socket);
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
