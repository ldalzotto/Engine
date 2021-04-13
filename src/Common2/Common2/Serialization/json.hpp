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

inline void remove_spaces(Vector<int8>& p_source)
{
    p_source.erase_all_elements_that_matches_element(' ');
    p_source.erase_all_elements_that_matches_element('\n');
    p_source.erase_all_elements_that_matches_element('\r');
    p_source.erase_all_elements_that_matches_element('\t');
};
}; // namespace JSONUtil

struct JSONDeserializer
{
    struct FieldNode
    {
        Slice<int8> value;
        Slice<int8> whole_field;
    };

    Slice<int8> source;

    Slice<int8> parent_cursor;
    Vector<FieldNode> stack_fields;
    uimax current_field;

    inline static JSONDeserializer allocate_default()
    {
        return JSONDeserializer{Slice<int8>::build_default(), Slice<int8>::build_default(), Vector<FieldNode>::allocate(0), (uimax)-1};
    };

    inline static JSONDeserializer allocate(const Slice<int8>& p_source, const Slice<int8>& p_parent_cursor)
    {
        return JSONDeserializer{p_source, p_parent_cursor, Vector<FieldNode>::allocate(0), (uimax)-1};
    };

    inline static JSONDeserializer start(Vector<int8>& p_source)
    {
        JSONUtil::remove_spaces(p_source);
        uimax l_start_index;
        Slice_find(p_source.to_slice(), slice_int8_build_rawstr("{"), &l_start_index);
        l_start_index += 1;
        return allocate(p_source.to_slice(), p_source.to_slice().slide_rv(l_start_index));
    };

    inline JSONDeserializer clone()
    {
        return JSONDeserializer{this->source, this->parent_cursor, Vector<FieldNode>::allocate_elements(this->stack_fields.to_slice()), this->current_field};
    };

    inline void free()
    {
        this->stack_fields.free();
        this->current_field = -1;
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

                this->stack_fields.push_back_element(l_field_node);
                this->current_field = this->stack_fields.Size - 1;
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

                this->stack_fields.push_back_element(l_field_node);
                this->current_field = this->stack_fields.Size - 1;
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

        Slice<int8> l_compared_slice = this->get_current_slice_cursor();

        String l_field_name_json = String::allocate(strlen(p_field_name) + 2);
        l_field_name_json.append(slice_int8_build_rawstr("\""));
        l_field_name_json.append(slice_int8_build_rawstr(p_field_name));
        l_field_name_json.append(slice_int8_build_rawstr("\":"));

        FieldNode l_field_node;
        if (find_next_json_field(l_compared_slice, l_field_name_json.to_slice(), '{', '}', &l_field_node.whole_field, &l_field_node.value))
        {
            l_field_node.value.slide(1); // for '{'
            *out_object_iterator = JSONDeserializer::allocate(this->source, l_field_node.value);

            this->stack_fields.push_back_element(l_field_node);
            this->current_field = this->stack_fields.Size - 1;
            l_field_found = 1;
        }

        l_field_name_json.free();

        return l_field_found;
    };

    inline int8 next_array(const int8* p_field_name, JSONDeserializer* out_object_iterator)
    {
        out_object_iterator->free();

        int8 l_field_found = 0;

        Slice<int8> l_compared_slice = this->get_current_slice_cursor();

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

            this->stack_fields.push_back_element(l_field_node);
            this->current_field = this->stack_fields.Size - 1;
            l_field_found = 1;
        }

        l_field_name_json.free();

        return l_field_found;
    };

    inline int8 next_array_object(JSONDeserializer* out_object_iterator)
    {

        out_object_iterator->free();

        int8 l_field_found = 0;

        Slice<int8> l_compared_slice = this->get_current_slice_cursor();

        Slice<int8> l_field_name_json_slice = slice_int8_build_rawstr("{");

        FieldNode l_field_node;
        if (find_next_json_field(l_compared_slice, l_field_name_json_slice, '{', '}', &l_field_node.whole_field, &l_field_node.value))
        {
            *out_object_iterator = JSONDeserializer::allocate(this->source, l_field_node.value);

            this->stack_fields.push_back_element(l_field_node);
            this->current_field = this->stack_fields.Size - 1;
            l_field_found = 1;
        }
        return l_field_found;
    };

    // TODO -> remove the out, we must query the current field instead
    inline int8 next_array_plain_value(Slice<int8>* out_plain_value)
    {
        int8 l_plain_value_found = 0;

        Slice<int8> l_compared_slice = this->get_current_slice_cursor();

        FieldNode l_field_node;
        if (find_next_json_array_plain_value(l_compared_slice, &l_field_node.whole_field, out_plain_value))
        {
            // We still push the plain value to the field stack so that the cursor is moving forward
            this->stack_fields.push_back_element(l_field_node);
            this->current_field = this->stack_fields.Size - 1;

            l_plain_value_found = 1;
        }

        return l_plain_value_found;
    };

    // TODO -> remove the out, we must query the current field instead
    inline int8 next_array_number_value(Slice<int8>* out_plain_value)
    {
        Slice<int8> l_compared_slice = this->get_current_slice_cursor();
        FieldNode l_field_node;
        if (find_next_json_array_number_value(l_compared_slice, &l_field_node.whole_field, out_plain_value))
        {
            this->stack_fields.push_back_element(l_field_node);
            this->current_field = this->stack_fields.Size - 1;
        }
        return 1;
    };

    inline FieldNode& get_currentfield()
    {
        return this->stack_fields.get(this->current_field);
    }

    inline static const Slice<int8> get_json_type(JSONDeserializer& p_deserializer)
    {
        p_deserializer.next_field("type");
        return p_deserializer.get_currentfield().value;
    };

  private:
    inline FieldNode* get_current_field()
    {
        if (this->current_field != (uimax)-1)
        {
            return &this->stack_fields.get(this->current_field);
        }
        return NULL;
    };

    inline int8 find_next_field_whole_value(Slice<int8>* out_field_whole_value)
    {

        *out_field_whole_value = this->get_current_slice_cursor();

        // If there is an unexpected int8acter, before the field name.
        // This can occur if the field is in the middle of a JSON Object.
        // This if statement is mendatory because if the field is at the first position of the JSON
        // object, then there is no unexpected int8acter. So we handle both cases;
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

    // TODO -> this is a total waste to recompute the cursor for every field, we must increment an internal counter for every add to the stack instead
    inline Slice<int8> get_current_slice_cursor()
    {
        Slice<int8> l_compared_slice = this->parent_cursor;
        if (this->current_field != (uimax)-1)
        {
            for (uimax i = 0; i < this->stack_fields.Size; i++)
            {
                l_compared_slice.slide(this->stack_fields.get(i).whole_field.Size);
                l_compared_slice.slide(1); // for getting after ","
            }
        }
        return l_compared_slice;
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

    inline static int8 find_next_json_array_plain_value(const Slice<int8>& p_source, Slice<int8>* out_plain_value_whole, Slice<int8>* out_plain_value_only)
    {
        if (p_source.compare(slice_int8_build_rawstr("\"")))
        {
            *out_plain_value_whole = p_source;
            Slice<int8> l_plain_value_with_trail = p_source.slide_rv(1);
            uimax l_end_index;
            if (Slice_find(l_plain_value_with_trail, slice_int8_build_rawstr("\""), &l_end_index))
            {
                *out_plain_value_only = Slice<int8>::build_begin_end(l_plain_value_with_trail.Begin, 0, l_end_index);
                out_plain_value_whole->Size = l_end_index + 2; // To add the "
                return 1;
            }
        }
        return 0;
    };

    inline static int8 find_next_json_array_number_value(const Slice<int8>& p_source, Slice<int8>* out_plain_value_whole, Slice<int8>* out_plain_value_only)
    {
        *out_plain_value_whole = p_source;
        for (loop(i, 0, p_source.Size))
        {
            if (p_source.get(i) == ',' || p_source.get(i) == ']')
            {
                *out_plain_value_only = Slice<int8>::build_begin_end(p_source.Begin, 0, i);
                out_plain_value_whole->Size = out_plain_value_only->Size;
                return 1;
            }
        }
        return 0;
    };
};

// TODO -> we want to have clean templated functions here instead of macros

#define json_deser_iterate_array_start(JsonFieldName, DeserializerVariable)                                                                                                                            \
    {                                                                                                                                                                                                  \
        JSONDeserializer l_array = JSONDeserializer::allocate_default(), l_object = JSONDeserializer::allocate_default();                                                                              \
        (DeserializerVariable)->next_array(JsonFieldName, &l_array);                                                                                                                                   \
        while (l_array.next_array_object(&l_object))                                                                                                                                                   \
        {

#define json_deser_iterate_array_end()                                                                                                                                                                 \
    }                                                                                                                                                                                                  \
    l_array.free();                                                                                                                                                                                    \
    l_object.free();                                                                                                                                                                                   \
    }

#define json_deser_iterate_array_object l_object

#define json_deser_object_field(FieldNameValue, DeserializerVariable, OutValueTypedVariable, FromString)                                                                                               \
    (DeserializerVariable)->next_field(FieldNameValue);                                                                                                                                                \
    OutValueTypedVariable = FromString((DeserializerVariable)->get_currentfield().value);

#define json_deser_object(ObjectFieldNameValue, DeserializerVariable, TmpDeserializerVariable, OutValueTypedVariable, FromSerializer)                                                                  \
    (DeserializerVariable)->next_object(ObjectFieldNameValue, TmpDeserializerVariable);                                                                                                                \
    OutValueTypedVariable = FromSerializer(TmpDeserializerVariable);

#if __DEBUG

#define json_get_type_checked(DeserializerVariable, AwaitedTypeSlice) JSONUtil::validate_json_type(JSONDeserializer::get_json_type(DeserializerVariable), AwaitedTypeSlice);

#else

#define json_get_type_checked(DeserializerVariable, AwaitedTypeSlice) JSONDeserializer::get_json_type(DeserializerVariable);

#endif

// TODO -> we don't want to use a string here because we may have preallocated a slice, instead, what we want is a generic algorithm that takes a ShadowString struct that expose the "append" method.
struct JSONSerializer
{
    String output;
    uimax current_indentation;

    inline static JSONSerializer allocate_default()
    {
        return JSONSerializer{String::allocate(0), 0};
    }

    inline void free()
    {
        this->output.free();
        this->current_indentation = 0;
    }

    inline void start()
    {
        this->output.append(slice_int8_build_rawstr("{\n"));
        this->current_indentation += 1;
    };

    inline void end()
    {
        this->remove_last_coma();
        this->output.append(slice_int8_build_rawstr("}"));
        this->current_indentation -= 1;
    };

    inline void push_field(const Slice<int8>& p_name, const Slice<int8>& p_value)
    {
        this->push_indentation();
        this->output.append(slice_int8_build_rawstr("\""));
        this->output.append(p_name);
        this->output.append(slice_int8_build_rawstr("\": \""));
        this->output.append(p_value);
        this->output.append(slice_int8_build_rawstr("\",\n"));
    };

    inline void start_object(const Slice<int8>& p_name)
    {
        this->push_indentation();
        this->output.append(slice_int8_build_rawstr("\""));
        this->output.append(p_name);
        this->output.append(slice_int8_build_rawstr("\": {\n"));
        this->current_indentation += 1;
    };

    inline void start_object()
    {
        this->push_indentation();
        this->output.append(slice_int8_build_rawstr("{\n"));
        this->current_indentation += 1;
    };

    inline void end_object()
    {
        this->remove_last_coma();
        this->current_indentation -= 1;
        this->push_indentation();
        this->output.append(slice_int8_build_rawstr("},\n"));
    };

    inline void start_array(const Slice<int8>& p_name)
    {
        this->push_indentation();
        this->output.append(slice_int8_build_rawstr("\""));
        this->output.append(p_name);
        this->output.append(slice_int8_build_rawstr("\": [\n"));
        this->current_indentation += 1;
    };

    inline void end_array()
    {
        this->remove_last_coma();
        this->current_indentation -= 1;
        this->push_indentation();
        this->output.append(slice_int8_build_rawstr("],\n"));
    };

  private:
    void push_indentation()
    {
        String l_indentation = String::allocate(this->current_indentation);
        for (size_t i = 0; i < this->current_indentation; i++)
        {
            l_indentation.append(slice_int8_build_rawstr(" "));
        }
        this->output.append(l_indentation.to_slice());
        l_indentation.free();
    };

    void remove_last_coma()
    {
        if (this->output.get(this->output.Memory.Size - 1 - 2) == ',')
        {
            this->output.erase_array_at(this->output.Memory.Size - 1 - 2, 1);
        }
    };
};
