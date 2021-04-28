#pragma once

// see https://tools.ietf.org/html/rfc6455#section-5.2 for specifications
struct WebSocketRequestReader
{

    inline static Slice<int8> read_body_not_compressed(const Slice<int8>& p_request)
    {
        // TODO -> handle opt
        int8 l_0 = p_request.get(0);
        assert_true((l_0 >> 7) & 0x1); // only one frame
        int8 l_1 = p_request.get(1);
        assert_true((l_1 >> 7 & 0x1)); // is masked

        uimax l_payload_size = (((l_1 >> 6) & 0x1) << 6) | (((l_1 >> 5) & 0x1) << 5) | (((l_1 >> 4) & 0x1) << 4) | (((l_1 >> 3) & 0x1) << 3) | (((l_1 >> 2) & 0x1) << 2) | (((l_1 >> 1) & 0x1) << 1) |
                               (((l_1 >> 0) & 0x1) << 0);

        int8 l_masking_key_byte_index = 2;

        if (l_payload_size == 126)
        {
            // TODO
            abort();
        }
        else if (l_payload_size == 127)
        {
            // TODO
            abort();
        };

        int8 l_m_0 = p_request.get(l_masking_key_byte_index);
        int8 l_m_1 = p_request.get(l_masking_key_byte_index + 1);
        int8 l_m_2 = p_request.get(l_masking_key_byte_index + 2);
        int8 l_m_3 = p_request.get(l_masking_key_byte_index + 3);

        SliceN<int8, 4> l_masking_key = {l_m_0, l_m_1, l_m_2, l_m_3};

        Slice<int8> l_payload_data = p_request.slide_rv(l_masking_key_byte_index + 4);
        l_payload_data.Size = l_payload_size;

        for (loop(i, 0, l_payload_data.Size))
        {
            l_payload_data.get(i) = l_payload_data.get(i) ^ l_masking_key.get(i % 4);
        }

        return l_payload_data;
    };

    inline static Slice<int8> add_websocket_data_to_buffer(const Slice<int8>& p_response_full_buffer, const Slice<int8>& p_response_only_buffer)
    {
        Slice<int8> l_target = p_response_full_buffer;
        int8 l_websocket_response_header_size;

        int8 l_fin_rsv_opt = 0x80 | 0x02; // only one frame; // opt code to binary
        int8 l_mask_payload_size = 0x00;  // not masked

        if (p_response_only_buffer.Size <= 0x7F)
        {
            l_mask_payload_size |= (int8)p_response_only_buffer.Size;
            l_websocket_response_header_size = 2;
            l_target.move_memory_down(p_response_only_buffer.Size, 0, l_websocket_response_header_size);

            l_target.get(0) = l_fin_rsv_opt;
            l_target.get(1) = l_mask_payload_size;
        }
        else if (p_response_only_buffer.Size <= 0xFFF)
        {
            // TODO
            abort();
        }
        else
        {
            // TODO
            abort();
        }

        l_target.Size = p_response_only_buffer.Size + l_websocket_response_header_size;

        return l_target;
    };
};

namespace WebSocketHandshake_Const
{
const int8* magic_number_arr = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
const Slice<int8> magic_number = slice_int8_build_rawstr(magic_number_arr);
}; // namespace WebSocketHandshake_Const

struct WebSocketHandshake
{
    int8 handshake_successful;

    inline static WebSocketHandshake build_default()
    {
        return WebSocketHandshake{0};
    };

    inline SocketRequestResponseConnection::ListenSendResponseReturnCode handle_handshake(const Slice<int8>& p_request, const Slice<int8>& p_response, Slice<int8>* p_response_slice)
    {
        if (p_request.compare(slice_int8_build_rawstr("GET /")))
        {
            uimax l_index;
            if (Slice_find(p_request, slice_int8_build_rawstr("Upgrade: websocket"), &l_index) && Slice_find(p_request, slice_int8_build_rawstr("Connection: Upgrade"), &l_index))
            {
                const int8* l_sec_key_header_prefix_raw = "Sec-WebSocket-Key: ";
                Slice<int8> l_sec_key_header_prefix = slice_int8_build_rawstr("Sec-WebSocket-Key: ");
                if (Slice_find(p_request, l_sec_key_header_prefix, &l_index))
                {
                    Slice<int8> l_key = p_request.slide_rv(l_index + l_sec_key_header_prefix.Size);
                    if (Slice_find(l_key, slice_int8_build_rawstr("\r\n"), &l_index))
                    {
                        l_key.Size = l_index;
                    }

                    Span<int8> l_key_with_magic_number = Span<int8>::allocate_slice_2(l_key, WebSocketHandshake_Const::magic_number);
                    SliceN<int8, 20> l_hashed_key = Hash_SHA1(l_key_with_magic_number.slice);
                    Span<int8> l_hashed_key_b64 = encode_base64(slice_from_slicen(&l_hashed_key));

                    // Span<int8> l_hashed_key = sha1_hash(l_key);

                    *p_response_slice = SocketRequestWriter::set_3(
                        slice_int8_build_rawstr("HTTP/1.1 101 Switching Protocols\r\nCompression: None\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Accept: "), l_hashed_key_b64.slice,
                        slice_int8_build_rawstr("\r\n\r\n"), p_response);

                    l_hashed_key_b64.free();
                    l_key_with_magic_number.free();

                    this->handshake_successful = 1;
                    return SocketRequestResponseConnection::ListenSendResponseReturnCode::SEND_RESPONSE;
                }
            }
        }

        return SocketRequestResponseConnection::ListenSendResponseReturnCode::NOTHING;
    };
};