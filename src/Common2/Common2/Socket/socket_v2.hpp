#pragma once

#define SOCKET_DEFAULT_PORT 8000

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
        socket_context_startup();
    };

    inline static void free()
    {
        socket_context_cleanup();
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
        socket_addrinfo_free(this->native_addr_info);
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
            this->accept_return.acceped_socket = socket_native_accept(this->native_socket);
            if (!this->accept_return.acceped_socket.is_valid())
            {
                if (!socket_native_is_error_is_blocking_type(socket_native_get_last_error()))
                {
                    this->accept_state.enabled = 0;
                }
            }
        }
    };

  private:
    inline static void _allocate_socket_and_bind_and_listen(SocketServerNonBlocking& p_socket, const int32 p_port)
    {
        p_socket.native_addr_info = socket_addrinfo_get(socket_native_type::TCP_SERVER, p_port);

        p_socket.native_socket = SocketNative::socket_allocate_bind(p_socket.native_addr_info);
        socket_native_set_non_blocking(p_socket.native_socket);

        socket_native_listen(p_socket.native_socket);
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
    socket_addrinfo native_addr_info;

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
        SocketClientNonBlocking l_socket;
        _allocate_socket_and_connect(l_socket, p_port);

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
        l_socket.native_addr_info.ptr = NULL;

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
        socket_addrinfo_free(this->native_addr_info);
        this->native_addr_info.ptr = NULL;
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
                int32 l_bytes_sent = socket_native_send(this->native_socket, this->send_input.sended_buffer.slide_rv(this->send_state.sended_bytes));

                if (l_bytes_sent == -1)
                {
                    if (!socket_native_is_error_is_blocking_type(socket_native_get_last_error()))
                    {
                        this->send_state.enabled = 0;
                    }
                }
                else
                {
                    if (l_bytes_sent == 0 || l_bytes_sent == -1)
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
            int32 l_bytes_received = socket_native_recv(this->native_socket, this->receive_input.target_buffer);

            if (l_bytes_received == -1)
            {
                int32 l_error = socket_native_get_last_error();
                if (!socket_native_is_error_is_blocking_type(l_error))
                {
                    // There is still data to receive
                    if (!socket_native_is_error_is_msgsize_type(l_error))
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
    inline static void _allocate_socket_and_connect(SocketClientNonBlocking& p_socket, const int32 p_port)
    {
        p_socket.native_addr_info = socket_addrinfo_get(socket_native_type::TCP_CLIENT, p_port);
        p_socket.native_socket = SocketNative::socket_allocate_connect(p_socket.native_addr_info);
        socket_native_set_non_blocking(p_socket.native_socket);
    };

    inline static void _initialize_states(SocketClientNonBlocking& p_socket)
    {
        p_socket.send_state.enabled = 0;

        p_socket.receive_state.enabled = 0;
    };
};
