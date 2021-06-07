import * as os from "os";

export function assert_true(p_condition: boolean) {
    if (!p_condition) {
        process.abort();
    }
}

export class MathConst {
    public static FLOAT_TOLERENCE: number = 0.0001;
}

export class MathC {
    public static equals_float(p_left: number, p_right: number) {
        return Math.abs(p_right - p_left) <= MathConst.FLOAT_TOLERENCE;
    }
};

export class ByteConst {
    public static is_little_endian: boolean = (os.endianness() == "LE");
    public static INT8: number = 1;
    public static INT32: number = 4;
    public static INT64: number = 8;
};

export class Slice {
    memory: ArrayBuffer;
    begin: number;
    end: number;

    public static from_memory(p_buffer: ArrayBuffer): Slice {
        let l_return = new Slice();
        l_return.memory = p_buffer;
        l_return.begin = 0;
        l_return.end = p_buffer.byteLength;
        return l_return;
    };

    public size(): number {
        return this.end - this.begin;
    };

    public slide_rv(p_element_offset: number): Slice {
        let l_slice = new Slice();
        l_slice.memory = this.memory;
        l_slice.begin = this.begin + p_element_offset;
        l_slice.end = this.end;
        assert_true(l_slice.begin <= this.end);
        return l_slice;
    };

    public compare(p_other: Slice): boolean {
        let l_size = this.size();
        for (let i = 0; i < l_size; i++) {

            if (this.memory[this.begin + i] != p_other.memory[p_other.begin + i]) {
                return false;
            }
        }
        return true;
    };

    public set_buffer(p_value: Slice, p_element_offset: number = 0) {
        new Int8Array(this.memory).set(new Int8Array(p_value.memory.slice(this.begin, this.end)), p_element_offset);
    };

    public set_array(p_value: ArrayBuffer, p_element_offset: number = 0) {
        new Int8Array(this.memory).set(new Int8Array(p_value), p_element_offset);
    };

    public set_int8(p_value: number, p_element_offset: number = 0) {
        new DataView(this.memory, this.begin + p_element_offset).setInt8(0, p_value);
    };

    public set_int32(p_value: number, p_element_offset: number = 0) {
        new DataView(this.memory, this.begin + p_element_offset).setInt32(0, p_value, ByteConst.is_little_endian);
    };

    public set_int64(p_value: number, p_element_offset: number = 0) {
        new Int8Array(this.memory, this.begin + p_element_offset).set(
            Int32Array.of(p_value, 0)
        );
    };

    public set_float32(p_value: number, p_element_offset: number = 0) {
        new DataView(this.memory, this.begin + p_element_offset).setFloat32(0, p_value, ByteConst.is_little_endian);
    };

    public get_int8(p_element_offset: number = 0): number {
        return new DataView(this.memory, this.begin + p_element_offset).getInt8(0);
    };

    public get_int32(p_element_offset: number = 0): number {
        return new DataView(this.memory, this.begin + p_element_offset).getInt32(0, ByteConst.is_little_endian);
    };

    public get_int64(p_element_offset: number = 0): number {
        return Number(new DataView(this.memory, this.begin + p_element_offset).getBigInt64(0, ByteConst.is_little_endian));
    };

    public get_float32(p_element_offset: number = 0): number {
        return new DataView(this.memory, this.begin + p_element_offset).getFloat32(0, ByteConst.is_little_endian);
    };

};

export class Span {
    memory: ArrayBuffer;
    begin: number;
    end: number;

    public size(): number {
        return this.to_slice().size();
    };

    public static allocate(p_size: number): Span {
        let l_span: Span = new Span();
        l_span.memory = new ArrayBuffer(p_size);
        l_span.begin = 0;
        l_span.end = p_size;
        return l_span;
    };

    public static from_string(p_str: string): Span {
        let l_span = Span.allocate(p_str.length);
        l_span.to_slice().set_array(new TextEncoder().encode(p_str));
        return l_span;
    };

    public to_slice(): Slice {
        let l_slice: Slice = new Slice();
        l_slice.memory = this.memory;
        l_slice.begin = this.begin;
        l_slice.end = this.end;
        return l_slice;
    };

    public resize(p_new_size: number) {
        if (p_new_size > this.size()) {
            let l_new_memory = new ArrayBuffer(p_new_size);
            new Int8Array(l_new_memory).set(new Int8Array(this.memory));
            this.memory = l_new_memory;
            let l_size_delta = p_new_size - this.size();
            this.end += l_size_delta;
        }
    };

};

export class Vector {
    span: Span;
    size: number;

    public static allocate(p_capacity: number): Vector {
        let l_vector = new Vector();
        l_vector.span = Span.allocate(p_capacity);
        l_vector.size = 0;
        return l_vector;
    };

    public push_back(p_slice: Slice) {
        this.fit_new_size(this.size + p_slice.size());
        this.span.to_slice().set_buffer(p_slice, this.size);
        this.size += p_slice.size();
    };

    public fit_new_size(p_new_size: number) {
        while (this.span.size() < p_new_size) {
            this.span.resize(this.span.size() == 0 ? 1 : (this.span.size() * 2));
        }
    };
};

export class Token<T> {
    tok: ArrayBuffer;

    static allocate_empty<T>(): Token<T> {
        return {tok: new ArrayBuffer(8)};
    };

    static build<T>(p_array_buffer: ArrayBuffer): Token<T> {
        return {tok: p_array_buffer};
    };

    static allocate<T>(p_array_buffer: ArrayBuffer): Token<T> {
        return {tok: p_array_buffer.slice(0)};
    };
};

export class BinaryDeserializer {

    public static int8(in_out_buffer: Slice): number {
        let l_return: number = in_out_buffer.get_int8();
        in_out_buffer.begin += ByteConst.INT8;
        return l_return;
    };

    public static int32(in_out_buffer: Slice): number {
        let l_return: number = in_out_buffer.get_int32();
        in_out_buffer.begin += ByteConst.INT32;
        return l_return;
    };

    public static int64(in_out_buffer: Slice): number {
        let l_return: number = in_out_buffer.get_int64();
        in_out_buffer.begin += ByteConst.INT64;
        return l_return;
    };

    public static float32(in_out_buffer: Slice): number {
        let l_return: number = in_out_buffer.get_float32();
        in_out_buffer.begin += ByteConst.INT32;
        return l_return;
    };


    public static slice(in_out_buffer: Slice): Slice {
        let l_return: Slice = new Slice();
        let l_size = this.int64(in_out_buffer);
        l_return.memory = in_out_buffer.memory;
        l_return.begin = in_out_buffer.begin;
        l_return.end = l_return.begin + l_size;
        return l_return;
    };

};

export class BinarySerializer {

    public static int64(in_out_buffer: Vector, p_value: number) {
        let l_span = Span.allocate(ByteConst.INT64);
        l_span.to_slice().set_int64(p_value);
        in_out_buffer.push_back(l_span.to_slice());
    };

    public static float32(in_out_buffer: Vector, p_value: number) {
        let l_span = Span.allocate(ByteConst.INT32);
        l_span.to_slice().set_float32(p_value);
        in_out_buffer.push_back(l_span.to_slice());
    };

    public static slice(in_out_buffer: Vector, p_slice: Slice) {
        BinarySerializer.int64(in_out_buffer, p_slice.size());
        in_out_buffer.push_back(p_slice);
    };

    public static string(in_out_buffer: Vector, p_str: string) {
        in_out_buffer.push_back(Span.from_string(p_str).to_slice());
    };
};