#pragma once

namespace ObjCompiler_Const
{
static const Slice<int8> vn = Slice_int8_build_rawstr("vn");
static const Slice<int8> vt = Slice_int8_build_rawstr("vt");
static const Slice<int8> v = Slice_int8_build_rawstr("v");
static const Slice<int8> f = Slice_int8_build_rawstr("f");
static const Slice<int8> space = Slice_int8_build_rawstr(" ");
static const Slice<int8> slash = Slice_int8_build_rawstr("/");
}; // namespace ObjCompiler_Const

struct ObjCompiler
{
    struct VertexKey
    {
        uimax position_index;
        uimax uv_index;

        uimax computed_indice;

        inline int8 equals(const VertexKey& p_other)
        {
            return this->position_index == p_other.position_index && this->uv_index == p_other.uv_index;
        }
    };

    inline static void ReadObj(const Slice<int8>& p_obj_content, Vector<Vertex>& out_vertices, Vector<uint32>& out_indices)
    {
        Vector<VertexKey> l_vertices_indexed = Vector<VertexKey>::allocate(0);
        Vector<v3f> l_positions = Vector<v3f>::allocate(0);
        Vector<v2f> l_uvs = Vector<v2f>::allocate(0);

        uimax l_begin_line_index = 0;
        for (loop(l_reader_index, 0, p_obj_content.Size))
        {
            if (*Slice_get(&p_obj_content, l_reader_index) == '\n')
            {
                Slice<int8> l_line = Slice_slide_rv(&p_obj_content, l_begin_line_index);
                l_line.Size = l_reader_index - l_begin_line_index;

                if (Slice_compare(&l_line, &ObjCompiler_Const::vn))
                {
                }
                else if (Slice_compare(&l_line, &ObjCompiler_Const::vt))
                {
                    v2f l_uv;

                    uimax l_first_space, l_second_space;
                    Slice_find(&l_line, &ObjCompiler_Const::space, &l_first_space);
                    l_first_space += 1;
                    Slice<int8> l_line_offsetted_first_space = Slice_slide_rv(&l_line, l_first_space);
                    Slice_find(&l_line_offsetted_first_space, &ObjCompiler_Const::space, &l_second_space);
                    l_second_space += l_first_space + 1;

                    l_uv.x = FromString::afloat32(Slice_build_begin_end<int8>(l_line.Begin, l_first_space, l_second_space - 1));
                    l_uv.y = 1.0f - FromString::afloat32(Slice_build_begin_end<int8>(l_line.Begin, l_second_space, l_line.Size));

                    l_uvs.push_back_element(l_uv);
                }
                else if (Slice_compare(&l_line, &ObjCompiler_Const::v))
                {
                    v3f l_local_position;
                    uimax l_first_space, l_second_space, l_third_space;
                    Slice_find(&l_line, &ObjCompiler_Const::space, &l_first_space);
                    l_first_space += 1;
                    Slice<int8> l_line_offsetted_first_space = Slice_slide_rv(&l_line, l_first_space);
                    Slice_find(&l_line_offsetted_first_space, &ObjCompiler_Const::space, &l_second_space);
                    l_second_space += l_first_space + 1;
                    Slice<int8> l_line_offsetted_second_space = Slice_slide_rv(&l_line, l_second_space);
                    Slice_find(&l_line_offsetted_second_space, &ObjCompiler_Const::space, &l_third_space);
                    l_third_space += l_second_space + 1;

                    l_local_position.x = FromString::afloat32(Slice_build_begin_end<int8>(l_line.Begin, l_first_space, l_second_space - 1));
                    l_local_position.y = FromString::afloat32(Slice_build_begin_end<int8>(l_line.Begin, l_second_space, l_third_space - 1));
                    l_local_position.z = FromString::afloat32(Slice_build_begin_end<int8>(l_line.Begin, l_third_space, l_line.Size));

                    l_positions.push_back_element(l_local_position);
                }

                else if (Slice_compare(&l_line, &ObjCompiler_Const::f))
                {
                    uimax l_first_space, l_second_space, l_third_space;
                    Slice_find(&l_line, &ObjCompiler_Const::space, &l_first_space);
                    l_first_space += 1;
                    Slice<int8> l_line_offsetted_first_line = Slice_slide_rv(&l_line, l_first_space);
                    Slice_find(&l_line_offsetted_first_line, &ObjCompiler_Const::space, &l_second_space);
                    l_second_space += l_first_space + 1;
                    Slice<int8> l_line_offsetted_second_line = Slice_slide_rv(&l_line, l_second_space);
                    Slice_find(&l_line_offsetted_second_line, &ObjCompiler_Const::space, &l_third_space);
                    l_third_space += l_second_space + 1;

                    process_obj_face(Slice_build_begin_end<int8>(l_line.Begin, l_first_space, l_second_space - 1), l_vertices_indexed, l_positions, l_uvs, out_vertices, out_indices);
                    process_obj_face(Slice_build_begin_end<int8>(l_line.Begin, l_second_space, l_third_space - 1), l_vertices_indexed, l_positions, l_uvs, out_vertices, out_indices);
                    process_obj_face(Slice_build_begin_end<int8>(l_line.Begin, l_third_space, l_line.Size), l_vertices_indexed, l_positions, l_uvs, out_vertices, out_indices);

                    // l_line.substr(l_first_space, l_second_space - l_first_space).
                }

                l_begin_line_index = l_reader_index + 1;
            }
        }

        l_vertices_indexed.free();
        l_positions.free();
        l_uvs.free();
    }

    inline static void process_obj_face(const Slice<int8> p_face, Vector<VertexKey>& p_vertices_indexed, Vector<v3f>& p_positions, Vector<v2f>& p_uvs, Vector<Vertex>& out_vertices,
                                        Vector<uint32>& out_indices)
    {
        uimax l_first_slash, l_second_slash;
        Slice_find(&p_face, &ObjCompiler_Const::slash, &l_first_slash);
        l_first_slash += 1;
        Slice<int8> l_face_offsetted_first_slash = Slice_slide_rv(&p_face, l_first_slash);
        Slice_find(&l_face_offsetted_first_slash, &ObjCompiler_Const::slash, &l_second_slash);
        l_second_slash += l_first_slash + 1;

        uimax l_position_index = FromString::auimax(Slice_build_begin_end<int8>(p_face.Begin, 0, l_first_slash - 1)) - 1;
        uimax l_uv_index = FromString::auimax(Slice_build_begin_end<int8>(p_face.Begin, l_first_slash, l_second_slash - 1)) - 1;

        VertexKey l_key;
        l_key.position_index = l_position_index;
        l_key.uv_index = l_uv_index;

        int8 l_key_already_indexed = 0;
        uimax l_key_already_indexed_index = 0;
        for (loop(i, 0, p_vertices_indexed.Size))
        {
            if (p_vertices_indexed.get(i).equals(l_key))
            {
                l_key_already_indexed = 1;
                l_key.computed_indice = i;
                break;
            }
        }

        if (!l_key_already_indexed)
        {
            l_key.computed_indice = p_vertices_indexed.Size;
            p_vertices_indexed.push_back_element(l_key);
            Vertex l_vertex;
            l_vertex.position = p_positions.get(l_position_index);
            l_vertex.uv = p_uvs.get(l_uv_index);
            out_vertices.push_back_element(l_vertex);
        }

        out_indices.push_back_element((uint32)l_key.computed_indice);
    }
};