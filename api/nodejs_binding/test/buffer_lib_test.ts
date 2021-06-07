import {assert_true, BinaryDeserializer, BinarySerializer, ByteConst, MathC, Span, Vector} from "../src/buffer_lib";

function buffer_lib_test() {
    let l_span = Span.allocate(16);
    {
        let l_slice = l_span.to_slice();
        l_slice.set_int32(10, 0 * ByteConst.INT32);
        l_slice.set_int8(0, ByteConst.INT32 + 0);
        l_slice.set_int8(1, ByteConst.INT32 + 1);
        l_slice.set_int8(2, ByteConst.INT32 + 2);
        l_slice.set_int8(3, ByteConst.INT32 + 3);
        l_slice.set_float32(12.10, 2 * ByteConst.INT32);
        l_slice.set_int32(13, 3 * ByteConst.INT32);

        assert_true(BinaryDeserializer.int32(l_slice) == 10);
        assert_true(BinaryDeserializer.int8(l_slice) == 0);
        assert_true(BinaryDeserializer.int8(l_slice) == 1);
        assert_true(BinaryDeserializer.int8(l_slice) == 2);
        assert_true(BinaryDeserializer.int8(l_slice) == 3);
        assert_true(MathC.equals_float(BinaryDeserializer.float32(l_slice), 12.10));
        assert_true(BinaryDeserializer.int32(l_slice) == 13);
    }
    {
        assert_true(MathC.equals_float(l_span.to_slice().get_float32(2 * ByteConst.INT32), 12.10));
    }
};

function vector_test() {
    let l_vector = Vector.allocate(0);

    let l_insert = Span.allocate(ByteConst.INT64 * 4);
    l_insert.to_slice().set_int64(10, ByteConst.INT64 * 0);
    l_insert.to_slice().set_int64(11, ByteConst.INT64 * 1);
    l_insert.to_slice().set_int64(12, ByteConst.INT64 * 2);
    l_insert.to_slice().set_int64(13, ByteConst.INT64 * 3);

    l_vector.push_back(l_insert.to_slice());
    assert_true(l_vector.size == l_insert.size());

    {
        let l_slice = l_vector.span.to_slice();
        assert_true(BinaryDeserializer.int64(l_slice) == 10);
        assert_true(BinaryDeserializer.int64(l_slice) == 11);
        assert_true(BinaryDeserializer.int64(l_slice) == 12);
        assert_true(BinaryDeserializer.int64(l_slice) == 13);
    }

    l_vector.push_back(l_insert.to_slice());
    assert_true(l_vector.size == l_insert.size() * 2);

    {
        let l_slice = l_vector.span.to_slice();
        assert_true(BinaryDeserializer.int64(l_slice) == 10);
        assert_true(BinaryDeserializer.int64(l_slice) == 11);
        assert_true(BinaryDeserializer.int64(l_slice) == 12);
        assert_true(BinaryDeserializer.int64(l_slice) == 13);
        assert_true(BinaryDeserializer.int64(l_slice) == 10);
        assert_true(BinaryDeserializer.int64(l_slice) == 11);
        assert_true(BinaryDeserializer.int64(l_slice) == 12);
        assert_true(BinaryDeserializer.int64(l_slice) == 13);
    }
};

function string_test() {
    let l_vector = Vector.allocate(0);
    let l_str = "hello, world!";
    BinarySerializer.slice(l_vector, Span.from_string(l_str).to_slice());
    assert_true(l_vector.size == (ByteConst.INT64 + l_str.length));
    {
        assert_true(l_vector.span.to_slice().get_int64(0) == l_str.length);

        let l_offset = ByteConst.INT64;
        let l_compare = new TextEncoder().encode(l_str);
        for (let i = 0; i < l_compare.byteLength; i++) {
            assert_true(l_vector.span.to_slice().get_int8(l_offset + i) == l_compare[i]);
        }
    }
};

function slice_serialization_test() {
    let l_span = Span.allocate(16);
    {
        let l_slice = l_span.to_slice();
        l_slice.set_int32(10, 0 * ByteConst.INT32);
        l_slice.set_float32(12.10, 1 * ByteConst.INT32);
        l_slice.set_int32(13, 2 * ByteConst.INT32);
    }

    let l_buffer: Vector = Vector.allocate(0);
    BinarySerializer.slice(l_buffer, l_span.to_slice());
    assert_true(l_buffer.size == (ByteConst.INT64 + l_span.size()));
    assert_true(l_buffer.span.to_slice().get_int64(0) == l_span.size());

    let l_vector_value_slice= l_buffer.span.to_slice().slide_rv(ByteConst.INT64);
    assert_true(l_vector_value_slice.compare(l_span.to_slice()));
};


buffer_lib_test();
vector_test();
string_test();
slice_serialization_test();