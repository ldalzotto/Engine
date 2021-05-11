#pragma once

namespace JSONUtil
{
inline Slice<int8> validate_json_type(const Slice<int8>& p_type, const Slice<int8>& p_awaited_type)
{
#if __DEBUG
    assert_true(p_type.compare(p_awaited_type));
#endif
    return p_type;
};

template <class ShadowVector(int8)> inline void sanitize_json(ShadowVector(int8) & p_source)
{
    sv_static_assert_element_type(ShadowVector(int8), int8);

    SliceN<int8, 4> l_chars_removed = {' ', '\n', '\r', '\t'};
    VectorAlgorithm::erase_all_elements_that_matches_any_of_element(p_source, slice_from_slicen(&l_chars_removed), Equality::Default{});
};
}; // namespace JSONUtil

struct JSONDeserializer
{
    struct FieldNode
    {
        Slice<int8> value;
        Slice<int8> whole_field;

        inline static FieldNode build_default()
        {
            FieldNode l_return;
            l_return.value = Slice<int8>::build_default();
            l_return.whole_field = Slice<int8>::build_default();
            return l_return;
        };
    };

    Slice<int8> source;

    Slice<int8> parent_cursor;
    FieldNode current_field;
    Slice<int8> deserialization_cursor;

    inline static JSONDeserializer allocate_default()
    {
        JSONDeserializer l_return;
        l_return.source = Slice<int8>::build_default();
        l_return.parent_cursor = Slice<int8>::build_default();
        l_return.current_field = FieldNode::build_default();
        l_return.deserialization_cursor = Slice<int8>::build_default();
        return l_return;
    };

    inline static JSONDeserializer allocate(const Slice<int8>& p_source, const Slice<int8>& p_parent_cursor)
    {
        JSONDeserializer l_return;
        l_return.source = p_source;
        l_return.parent_cursor = p_parent_cursor;
        l_return.current_field = FieldNode::build_default();
        l_return.deserialization_cursor = p_parent_cursor;
        return l_return;
    };

    inline static JSONDeserializer start(const Slice<int8>& p_source)
    {
        uimax l_start_index;
        Slice_find(p_source, slice_int8_build_rawstr("{"), &l_start_index);
        l_start_index += 1;
        return allocate(p_source, p_source.slide_rv(l_start_index));
    };

    template <class ShadowVector(int8)> inline static JSONDeserializer sanitize_and_start(ShadowVector(int8) & p_source)
    {
        sv_static_assert_element_type(ShadowVector(int8), int8);

        JSONUtil::sanitize_json(p_source);
        return JSONDeserializer::start(p_source.to_slice());
    };

    inline JSONDeserializer clone()
    {
        return *this;
    };

    inline void free(){
        //  this->stack_fields.free();
    };

    inline int8 next_field(const int8* p_field_name)
    {
        int8 l_field_found = 0;
        Slice<int8> l_next_field_whole_value;
        if (this->find_next_field_whole_value(&l_next_field_whole_value))
        {
            String l_field_name_json = String::allocate(strlen(p_field_name) + 2);
            l_field_name_json.append(slice_int8_build_rawstr("\""));
            l_field_name_json.append(slice_int8_build_rawstr(p_field_name));
            l_field_name_json.append(slice_int8_build_rawstr("\":"));

            if (l_next_field_whole_value.compare(l_field_name_json.to_slice()))
            {
                Slice<int8> l_next_field_value_with_quotes = l_next_field_whole_value.slide_rv(l_field_name_json.get_length());
                // To remove quotes
                l_next_field_value_with_quotes.slide(1);
                l_next_field_value_with_quotes.Size -= 1;

                FieldNode l_field_node;

                l_field_node.value = l_next_field_value_with_quotes;
                l_field_node.whole_field = l_next_field_whole_value;

                this->push_field_to_stack(l_field_node);
                l_field_found = 1;
            }

            l_field_name_json.free();
        };

        return l_field_found;
    };

    inline int8 next_number(const int8* p_field_name)
    {
        int8 l_field_found = 0;
        Slice<int8> l_next_field_whole_value;
        if (this->find_next_field_whole_value(&l_next_field_whole_value))
        {
            String l_field_name_json = String::allocate(strlen(p_field_name) + 2);
            l_field_name_json.append(slice_int8_build_rawstr("\""));
            l_field_name_json.append(slice_int8_build_rawstr(p_field_name));
            l_field_name_json.append(slice_int8_build_rawstr("\":"));

            if (l_next_field_whole_value.compare(l_field_name_json.to_slice()))
            {
                Slice<int8> l_next_field_value = l_next_field_whole_value.slide_rv(l_field_name_json.get_length());

                FieldNode l_field_node;

                l_field_node.value = l_next_field_value;
                l_field_node.whole_field = l_next_field_whole_value;

                this->push_field_to_stack(l_field_node);
                l_field_found = 1;
            }

            l_field_name_json.free();
        };

        return l_field_found;
    };

    inline int8 next_object(const int8* p_field_name, JSONDeserializer* out_object_iterator)
    {
        out_object_iterator->free();

        int8 l_field_found = 0;

        Slice<int8> l_compared_slice = this->deserialization_cursor;

        String l_field_name_json = String::allocate(strlen(p_field_name) + 2);
        l_field_name_json.append(slice_int8_build_rawstr("\""));
        l_field_name_json.append(slice_int8_build_rawstr(p_field_name));
        l_field_name_json.append(slice_int8_build_rawstr("\":"));

        FieldNode l_field_node;
        if (find_next_json_field(l_compared_slice, l_field_name_json.to_slice(), '{', '}', &l_field_node.whole_field, &l_field_node.value))
        {
            l_field_node.value.slide(1); // for '{'
            *out_object_iterator = JSONDeserializer::allocate(this->source, l_field_node.value);

            this->push_field_to_stack(l_field_node);
            l_field_found = 1;
        }

        l_field_name_json.free();

        return l_field_found;
    };

    inline int8 next_array(const int8* p_field_name, JSONDeserializer* out_object_iterator)
    {
        out_object_iterator->free();

        int8 l_field_found = 0;

        Slice<int8> l_compared_slice = this->deserialization_cursor;

        String l_field_name_json = String::allocate(strlen(p_field_name) + 2);
        l_field_name_json.append(slice_int8_build_rawstr("\""));
        l_field_name_json.append(slice_int8_build_rawstr(p_field_name));
        l_field_name_json.append(slice_int8_build_rawstr("\":"));

        FieldNode l_field_node;
        if (find_next_json_field(l_compared_slice, l_field_name_json.to_slice(), '[', ']', &l_field_node.whole_field, &l_field_node.value))
        {
            // To skip the first "["
            l_field_node.whole_field.Begin += 1;
            l_field_node.value.Begin += 1;

            *out_object_iterator = JSONDeserializer::allocate(this->source, l_field_node.value);

            this->push_field_to_stack(l_field_node);
            l_field_found = 1;
        }

        l_field_name_json.free();

        return l_field_found;
    };

    inline int8 next_array_object(JSONDeserializer* out_object_iterator)
    {

        out_object_iterator->free();

        int8 l_field_found = 0;

        Slice<int8> l_compared_slice = this->deserialization_cursor;

        Slice<int8> l_field_name_json_slice = slice_int8_build_rawstr("{");

        FieldNode l_field_node;
        if (find_next_json_field(l_compared_slice, l_field_name_json_slice, '{', '}', &l_field_node.whole_field, &l_field_node.value))
        {
            *out_object_iterator = JSONDeserializer::allocate(this->source, l_field_node.value);

            this->push_field_to_stack(l_field_node);
            l_field_found = 1;
        }
        return l_field_found;
    };

    inline int8 next_array_string_value(Slice<int8>* out_plain_value)
    {
        int8 l_plain_value_found = 0;

        Slice<int8> l_compared_slice = this->deserialization_cursor;

        FieldNode l_field_node;
        if (find_next_json_array_string_value(l_compared_slice, &l_field_node.whole_field, out_plain_value))
        {
            // We still push the plain value to the field stack so that the cursor is moving forward
            this->push_field_to_stack(l_field_node);

            l_plain_value_found = 1;
        }

        return l_plain_value_found;
    };

    inline int8 next_array_number_value(Slice<int8>* out_plain_value)
    {
        Slice<int8> l_compared_slice = this->deserialization_cursor;
        FieldNode l_field_node;
        if (find_next_json_array_number_value(l_compared_slice, &l_field_node.whole_field, out_plain_value))
        {
            this->push_field_to_stack(l_field_node);
        }
        return 1;
    };

    inline FieldNode& get_currentfield()
    {
        return this->current_field;
        // return this->stack_fields.get(this->stack_fields.Size - 1);
    }

    inline static const Slice<int8> get_json_type(JSONDeserializer& p_deserializer)
    {
        p_deserializer.next_field("type");
        return p_deserializer.get_currentfield().value;
    };

  private:
    inline int8 find_next_field_whole_value(Slice<int8>* out_field_whole_value)
    {

        *out_field_whole_value = this->deserialization_cursor;

        // If there is an unexpected characte, before the field name.
        // This can occur if the field is in the middle of a JSON Object.
        // This if statement is mendatory because if the field is at the first position of the JSON
        // object, then there is no unexpected character. So we handle both cases;
        if (out_field_whole_value->Size > 0 && (out_field_whole_value->get(0) == ',' || out_field_whole_value->get(0) == '{'))
        {
            out_field_whole_value->slide(1);
        }

        // then we get the next field

        uimax l_new_field_index;

        if (Slice_find(*out_field_whole_value, slice_int8_build_rawstr(","), &l_new_field_index) || Slice_find(*out_field_whole_value, slice_int8_build_rawstr("}"), &l_new_field_index))
        {
            out_field_whole_value->Size = l_new_field_index;
            return 1;
        }

        out_field_whole_value->Size = 0;
        return 0;
    };

    inline void push_field_to_stack(const FieldNode& p_field)
    {
        this->current_field = p_field;
        this->deserialization_cursor.slide(p_field.whole_field.Size + 1); // the +1 is for getting after ","
    };

    inline static int8 find_next_json_field(const Slice<int8>& p_source, const Slice<int8>& p_field_name, const int8 value_begin_delimiter, const int8 value_end_delimiter,
                                            Slice<int8>* out_object_whole_field, Slice<int8>* out_object_value_only)
    {
        if (p_source.compare(p_field_name))
        {
            Slice<int8> l_object_value = p_source.slide_rv(p_field_name.Size);

            uimax l_openedbrace_count = 1;
            uimax l_object_string_iterator = 1;
            while (l_openedbrace_count != 0 && l_object_string_iterator < l_object_value.Size)
            {
                if (l_object_value.get(l_object_string_iterator) == value_begin_delimiter)
                {
                    l_openedbrace_count += 1;
                }
                else if (l_object_value.get(l_object_string_iterator) == value_end_delimiter)
                {
                    l_openedbrace_count -= 1;
                }

                l_object_string_iterator += 1;
            }

            l_object_value.Size = l_object_string_iterator;

            *out_object_value_only = l_object_value;
            // out_object_value_only->Size = l_object_string_iterator - 1;
            *out_object_whole_field = p_source;
            out_object_whole_field->Size = p_field_name.Size + l_object_value.Size;

            return 1;
        }

        return 0;
    };

    inline static int8 find_next_json_array_string_value(const Slice<int8>& p_source, Slice<int8>* out_string_value_with_quotes, Slice<int8>* out_string_value)
    {
        if (p_source.compare(slice_int8_build_rawstr("\"")))
        {
            *out_string_value_with_quotes = p_source;
            Slice<int8> l_plain_value_with_trail = p_source.slide_rv(1);
            uimax l_end_index;
            if (Slice_find(l_plain_value_with_trail, slice_int8_build_rawstr("\""), &l_end_index))
            {
                *out_string_value = Slice<int8>::build_begin_end(l_plain_value_with_trail.Begin, 0, l_end_index);
                out_string_value_with_quotes->Size = l_end_index + 2; // To add the "
                return 1;
            }
        }
        return 0;
    };

    inline static int8 find_next_json_array_number_value(const Slice<int8>& p_source, Slice<int8>* out_number_value_with_quotes, Slice<int8>* out_number_value)
    {
        *out_number_value_with_quotes = p_source;
        for (loop(i, 0, p_source.Size))
        {
            if (p_source.get(i) == ',' || p_source.get(i) == ']')
            {
                *out_number_value = Slice<int8>::build_begin_end(p_source.Begin, 0, i);
                out_number_value_with_quotes->Size = out_number_value->Size;
                return 1;
            }
        }
        return 0;
    };
};

#define json_deser_iterate_array_object l_object

#if __DEBUG

#define json_get_type_checked(DeserializerVariable, AwaitedTypeSlice) JSONUtil::validate_json_type(JSONDeserializer::get_json_type(DeserializerVariable), AwaitedTypeSlice);

#else

#define json_get_type_checked(DeserializerVariable, AwaitedTypeSlice) JSONDeserializer::get_json_type(DeserializerVariable);

#endif

template <class ShadowString(_)> struct JSONSerializer
{
    ShadowString(_) & output;

    inline static JSONSerializer build(ShadowString(_) & p_shadow_string)
    {
        return JSONSerializer{p_shadow_string};
    }

    inline void start()
    {
        ShadowString_c_append(this->output, slice_int8_build_rawstr("{"));
    };

    inline void end()
    {
        ShadowString_c_append(this->output, slice_int8_build_rawstr("}"));
    };

    inline void coma()
    {
        ShadowString_c_append(this->output, slice_int8_build_rawstr(","));
    };

    inline void push_field(const Slice<int8>& p_name, const Slice<int8>& p_value)
    {
        ShadowString_c_append(this->output, slice_int8_build_rawstr("\""));
        ShadowString_c_append(this->output, p_name);
        ShadowString_c_append(this->output, slice_int8_build_rawstr("\":\""));
        ShadowString_c_append(this->output, p_value);
        ShadowString_c_append(this->output, slice_int8_build_rawstr("\""));
    };

    inline void start_object(const Slice<int8>& p_name)
    {
        ShadowString_c_append(this->output, slice_int8_build_rawstr("\""));
        ShadowString_c_append(this->output, p_name);
        ShadowString_c_append(this->output, slice_int8_build_rawstr("\":{"));
    };

    inline void start_object()
    {
        ShadowString_c_append(this->output, slice_int8_build_rawstr("{"));
    };

    inline void end_object()
    {
        ShadowString_c_append(this->output, slice_int8_build_rawstr("}"));
    };

    inline void start_array(const Slice<int8>& p_name)
    {
        ShadowString_c_append(this->output, slice_int8_build_rawstr("\""));
        ShadowString_c_append(this->output, p_name);
        ShadowString_c_append(this->output, slice_int8_build_rawstr("\":["));
    };

    inline void push_array_number(const Slice<int8>& p_number)
    {
        ShadowString_c_append(this->output, p_number);
    };

    inline void push_array_field(const Slice<int8>& p_number)
    {
        ShadowString_c_append(this->output, slice_int8_build_rawstr("\""));
        ShadowString_c_append(this->output, p_number);
        ShadowString_c_append(this->output, slice_int8_build_rawstr("\""));
    };

    inline void end_array()
    {
        ShadowString_c_append(this->output, slice_int8_build_rawstr("]"));
    };
};
