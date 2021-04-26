#pragma once


namespace MaterialViewerDomain
{
struct EngineThreadStartInputJSON
{
    Slice<int8> database;

    inline static void serialize(JSONSerializer& p_json, const Slice<int8>& p_database)
    {
        p_json.push_field(slice_int8_build_rawstr("database"), p_database);
    };

    inline static EngineThreadStartInputJSON deserialize(JSONDeserializer& p_json)
    {
        EngineThreadStartInputJSON l_return;
        p_json.next_field("database");
        l_return.database = p_json.get_currentfield().value;
        return l_return;
    };
};

struct GetAllMaterialsOutputJSON
{
    Vector<Span<int8>> materials;

    inline void free()
    {
        for (loop(i, 0, this->materials.Size))
        {
            this->materials.get(i).free();
        }
        this->materials.free();
    };

    inline static GetAllMaterialsOutputJSON deserialize(JSONDeserializer& p_json)
    {
        GetAllMaterialsOutputJSON l_return;
        JSONDeserializer l_materials_array_deserializer = JSONDeserializer::allocate_default();
        p_json.next_array("materials", &l_materials_array_deserializer);

        // TODO -> having a way to get the number of elements in a json array ?
        l_return.materials = Vector<Span<int8>>::allocate(0);
        Slice<int8> l_material_str;
        while (l_materials_array_deserializer.next_array_plain_value(&l_material_str))
        {
            l_return.materials.push_back_element(Span<int8>::allocate_slice(l_material_str));
        }
        l_materials_array_deserializer.free();

        return l_return;
    };

    inline static void serialize_from_path(JSONSerializer& p_json, const Vector<Span<int8>>& p_paths)
    {
        p_json.start_array(slice_int8_build_rawstr("materials"));
        for (loop(i, 0, p_paths.Size))
        {
            p_json.push_array_field(p_paths.get(i).slice);
        }
        p_json.end_array();
    };
};

struct GetAllMeshesOutputJSON
{
    Vector<Span<int8>> meshes;

    inline void free()
    {
        for (loop(i, 0, this->meshes.Size))
        {
            this->meshes.get(i).free();
        }
        this->meshes.free();
    };

    inline static GetAllMeshesOutputJSON deserialize(JSONDeserializer& p_json)
    {
        GetAllMeshesOutputJSON l_return;
        JSONDeserializer l_materials_array_deserializer = JSONDeserializer::allocate_default();
        p_json.next_array("meshes", &l_materials_array_deserializer);

        // TODO -> having a way to get the number of elements in a json array ?
        l_return.meshes = Vector<Span<int8>>::allocate(0);
        Slice<int8> l_material_str;
        while (l_materials_array_deserializer.next_array_plain_value(&l_material_str))
        {
            l_return.meshes.push_back_element(Span<int8>::allocate_slice(l_material_str));
        }
        l_materials_array_deserializer.free();

        return l_return;
    };

    inline static void serialize_from_path(JSONSerializer& p_json, const Vector<Span<int8>>& p_paths)
    {
        p_json.start_array(slice_int8_build_rawstr("meshes"));
        for (loop(i, 0, p_paths.Size))
        {
            p_json.push_array_field(p_paths.get(i).slice);
        }
        p_json.end_array();
    };
};

struct SetMaterialAndMeshInputJSON
{
    Slice<int8> material;
    Slice<int8> mesh;

    inline static SetMaterialAndMeshInputJSON deserialize(JSONDeserializer& p_json)
    {
        SetMaterialAndMeshInputJSON l_return;
        p_json.next_field("material");
        l_return.material = p_json.get_currentfield().value;
        p_json.next_field("mesh");
        l_return.mesh = p_json.get_currentfield().value;
        return l_return;
    };

    inline void serialize(JSONSerializer& p_json) const
    {
        p_json.push_field(slice_int8_build_rawstr("material"), this->material);
        p_json.push_field(slice_int8_build_rawstr("mesh"), this->mesh);
    };
};
}; // namespace MaterialViewerDomain