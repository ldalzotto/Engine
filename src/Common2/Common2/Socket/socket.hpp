#pragma once

#include <winsock2.h>
#include <ws2tcpip.h>
// #include <iphlpapi.h>
// #include <bcrypt.h>
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")
#pragma comment(lib, "AdvApi32.lib")

#define SOCKET_DEFAULT_PORT 8000

// TODO
// Move socket to common (with a preprocess condition ?)
// Create the win32 socket client
// Test it

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
    Vector<socket_t> sockets;

    inline static SocketContext allocate()
    {
        WSADATA l_winsock_data;
        winsock_error_handler(WSAStartup(MAKEWORD(2, 2), &l_winsock_data));
        return SocketContext{Vector<socket_t>::allocate(0)};
    };

    inline void free()
    {
#if __DEBUG
        assert_true(this->sockets.empty());
#endif
        this->sockets.free();
        WSACleanup();
    };

    inline void remove_socket(const socket_t p_socket)
    {
        for (loop(i, 0, this->sockets.Size))
        {
            if (this->sockets.get(i) == p_socket)
            {
                this->sockets.erase_element_at_always(i);
                break;
            }
        }
    };
};

inline socket_t socket_allocate(SocketContext& p_ctx, const socket_t p_socket)
{
#if __MEMLEAK
    push_ptr_to_tracked((int8*)p_socket);
#endif
#if __DEBUG
    assert_true(p_socket != INVALID_SOCKET);
#endif
    p_ctx.sockets.push_back_element(p_socket);
    return p_socket;
};

inline void socket_free(SocketContext& p_ctx, socket_t* p_socket)
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

enum class SocketReturnCode
{
    IDLING = 0,
    GRACEFULLY_CLOSED = 1
};

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

struct SocketServer
{
    socket_t client_listening_socket;
    socket_t client_socket;

    inline static SocketServer allocate(SocketContext& p_ctx, const int32 p_port)
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

#if 0
        sockaddr_in addr {};
        addr.sin_family = AF_INET;
        addr.sin_port = p_port;
        addr.sin_addr.s_addr = INADDR_ANY;
#endif
        l_return.client_listening_socket = socket_allocate(p_ctx, socket(l_result->ai_family, l_result->ai_socktype, l_result->ai_protocol));

        winsock_error_handler(bind(l_return.client_listening_socket, l_result->ai_addr, (int)l_result->ai_addrlen));

        freeaddrinfo(l_result);

        return l_return;
    };

    inline void free(SocketContext& p_ctx)
    {
        socket_free(p_ctx, &this->client_socket);
        socket_free(p_ctx, &this->client_listening_socket);
    };

    inline void wait_for_client(SocketContext& p_ctx)
    {
        winsock_error_handler(listen(this->client_listening_socket, SOMAXCONN));
        socket_t cc = accept(this->client_listening_socket, NULL, NULL);
        this->client_socket = socket_allocate(p_ctx, cc);
    };
};

struct SocketClient
{
    socket_t client_socket;

    inline static SocketClient allocate(SocketContext& p_ctx, const int32 p_port)
    {
        SocketClient l_return;

        int8 p_port_str_raw[ToString::int32str_size];
        ToString::aint32(p_port, slice_int8_build_rawstr(p_port_str_raw));

        addrinfo l_addr_info{};
        l_addr_info.ai_family = AF_INET;
        l_addr_info.ai_socktype = SOCK_STREAM;
        l_addr_info.ai_protocol = IPPROTO_TCP;
        // l_addr_info.ai_flags = AI_PASSIVE;

        addrinfo* l_result = NULL;

        winsock_error_handler(getaddrinfo(NULL, p_port_str_raw, &l_addr_info, &l_result));

#if 0
        l_return.client_socket = socket_allocate(p_ctx, socket(l_result->ai_family, l_result->ai_socktype, l_result->ai_protocol));
        winsock_error_handler(connect(l_return.client_socket, l_result->ai_addr, (int)l_result->ai_addrlen));
#endif
#if 1
        addrinfo* l_ptr;
        for (l_ptr = l_result; l_ptr != NULL; l_ptr = l_ptr->ai_next)
        {
            l_return.client_socket = socket_allocate(p_ctx, socket(l_result->ai_family, l_result->ai_socktype, l_result->ai_protocol));
            if (connect(l_return.client_socket, l_result->ai_addr, (int)l_result->ai_addrlen) != 0)
            {
                socket_free(p_ctx, &l_return.client_socket);
                continue;
            }
            else
            {
                break;
            }
        };
        if (l_ptr == NULL)
        {
            abort();
        }
#endif
        freeaddrinfo(l_result);
        return l_return;
    };

    inline void free(SocketContext& p_ctx)
    {
        socket_free(p_ctx, &this->client_socket);
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

struct SocketCommunication
{
    enum class RequestListenerReturnCode : int8
    {
        NOTHING = 0,
        SEND_RESPONSE = 1,
        ABORT_LISTENER = 2,
        SEND_RESPONSE_AND_ABORT_LISTENER = SEND_RESPONSE | ABORT_LISTENER
    };

    // TODO -> add variants that simulates the RequestListenerReturnCode and simplify the API
    template <class RequestCallbackFunc> inline static void listen_for_requests(SocketContext& p_ctx, socket_t p_socket, socket_t p_response_socket, const RequestCallbackFunc& p_request_callback_func)
    {
        SliceN<int8, 512> l_buffer = {};
        Slice<int8> l_buffer_slice = slice_from_slicen(&l_buffer);
        SliceN<int8, 512> l_response_buffer = {};
        Slice<int8> l_response_slice = slice_from_slicen(&l_response_buffer);

        int8 l_result = 1;

        while (l_result)
        {
            SocketReturnCode l_receive_return = socket_receive(p_socket, l_buffer.Memory, l_buffer.Size());

            RequestListenerReturnCode l_return_code = p_request_callback_func(l_buffer_slice, l_response_slice);
            if ((int8)l_return_code & (int8)(RequestListenerReturnCode::SEND_RESPONSE))
            {
                socket_send(p_response_socket, l_response_slice.Begin, l_response_slice.Size);
            }
            if (((int8)l_return_code & (int8)(RequestListenerReturnCode::ABORT_LISTENER)) || (l_receive_return == SocketReturnCode::GRACEFULLY_CLOSED))
            {
                l_result = 0;
            }
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
    };

    inline static SocketReturnCode send(socket_t p_socket, const Slice<int8>& p_socket_buffer, const SocketRequest& p_request)
    {
        Slice<int8> l_socket_buffer = p_socket_buffer;
        BinarySerializer::type<int32>(&l_socket_buffer, p_request.code);
        BinarySerializer::slice(&l_socket_buffer, p_request.payload);

        return socket_send(p_socket, p_socket_buffer.Begin, p_socket_buffer.Size - l_socket_buffer.Size);
    };
};
