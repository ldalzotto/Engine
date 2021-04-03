#pragma once

namespace AssetMetadataDatabase_Const
{
static const int8* DB_ASSET_METADATA_TABLE_INITIALIZATION =
    MULTILINE(create table if not exists asset_metadata(id integer PRIMARY KEY, path text, type text, file_modification_ts integer, insert_ts integer););
    static const int8* ASSET_METADATA_INSERT_QUERY = MULTILINE(insert into asset_metadata(id, path, type, file_modification_ts, insert_ts) values(?,?,?,?,?););
    static const int8* ASSET_METADATA_UPDATE_QUERY = MULTILINE(update asset_metadata set path = ?, type = ?, file_modification_ts = ?, insert_ts = ? where asset_metadata.id = ?;);
    static const int8* ASSET_METADATA_COUNT_QUERY = MULTILINE(select count(*) from asset_metadata where asset_metadata.id = ?;);
    static const int8* ASSET_METADATA_SELECT_QUERY = MULTILINE(select path, type, file_modification_ts, insert_ts from asset_metadata where asset_metadata.id = ?;);
    static const int8* ASSET_METADATA_SELECT_PATHS_FROM_TYPE = MULTILINE(select path from asset_metadata where asset_metadata.type = ?;);
    static const int8* ASSET_METADATA_SELECT_TIMESTAMPS = MULTILINE(select file_modification_ts, insert_ts from asset_metadata where asset_metadata.id = ?;);
    }; // namespace AssetMetadataDatabase_Const

    struct AssetMetadataDatabase
    {
        SQLitePreparedQuery assetmetadata_insert_query;
        SQLitePreparedQuery assetmetadata_update_query;
        SQLitePreparedQuery assetmetadata_select_query;
        SQLitePreparedQuery assetmetadata_count_query;
        SQLitePreparedQuery assetmetadata_select_paths_from_type;
        SQLitePreparedQuery assetmetadata_select_timestamps;

        inline static void initialize_database(DatabaseConnection& p_database_connection)
        {
            SQLiteQuery l_query = SQLiteQuery::allocate(p_database_connection, Slice_int8_build_rawstr(AssetMetadataDatabase_Const::DB_ASSET_METADATA_TABLE_INITIALIZATION));
            SQliteQueryExecution::execute_sync(p_database_connection, l_query.statement, []() {
            });
            l_query.free(p_database_connection);
        };

        inline static AssetMetadataDatabase allocate(DatabaseConnection& p_database_connection)
        {
            Slice_declare_sized(SQLiteQueryPrimitiveTypes, 1, tmp_layout_int64, tmp_layout_int64_slice, SQLiteQueryPrimitiveTypes::INT64);
            Slice_declare_sized(SQLiteQueryPrimitiveTypes, 2, tmp_layout_int64_int64, tmp_layout_int64_int64_slice, SQLiteQueryPrimitiveTypes::INT64, SQLiteQueryPrimitiveTypes::INT64);
            Slice_declare_sized(SQLiteQueryPrimitiveTypes, 1, tmp_layout_text, tmp_layout_text_slice, SQLiteQueryPrimitiveTypes::TEXT);
            Slice_declare_sized(SQLiteQueryPrimitiveTypes, 4, tmp_layout_text_text_int64_int64, tmp_layout_text_text_int64_int64_slice, SQLiteQueryPrimitiveTypes::TEXT,
                                SQLiteQueryPrimitiveTypes::TEXT, SQLiteQueryPrimitiveTypes::INT64, SQLiteQueryPrimitiveTypes::INT64);
            Slice_declare_sized(SQLiteQueryPrimitiveTypes, 5, tmp_layout_int64_text_text_int64_int64, tmp_layout_int64_text_text_int64_int64_slice, SQLiteQueryPrimitiveTypes::INT64,
                                SQLiteQueryPrimitiveTypes::TEXT, SQLiteQueryPrimitiveTypes::TEXT, SQLiteQueryPrimitiveTypes::INT64, SQLiteQueryPrimitiveTypes::INT64);
            Slice_declare_sized(SQLiteQueryPrimitiveTypes, 5, tmp_layout_text_text_int64_int64_int64, tmp_layout_text_text_int64_int64_int64_slice, SQLiteQueryPrimitiveTypes::TEXT,
                                SQLiteQueryPrimitiveTypes::TEXT, SQLiteQueryPrimitiveTypes::INT64, SQLiteQueryPrimitiveTypes::INT64, SQLiteQueryPrimitiveTypes::INT64);

            AssetMetadataDatabase l_database;
            l_database.assetmetadata_insert_query =
                SQLitePreparedQuery::allocate(p_database_connection, Slice_int8_build_rawstr(AssetMetadataDatabase_Const::ASSET_METADATA_INSERT_QUERY),
                                              SQLiteQueryLayout::allocate_span(Span_allocate_slice(&tmp_layout_int64_text_text_int64_int64_slice)), SQLiteQueryLayout::build_default());
            l_database.assetmetadata_update_query =
                SQLitePreparedQuery::allocate(p_database_connection, Slice_int8_build_rawstr(AssetMetadataDatabase_Const::ASSET_METADATA_UPDATE_QUERY),
                                              SQLiteQueryLayout::allocate_span(Span_allocate_slice(&tmp_layout_text_text_int64_int64_int64_slice)), SQLiteQueryLayout::build_default());
            l_database.assetmetadata_select_query = SQLitePreparedQuery::allocate(p_database_connection, Slice_int8_build_rawstr(AssetMetadataDatabase_Const::ASSET_METADATA_SELECT_QUERY),
                                                                                  SQLiteQueryLayout::allocate_span(Span_allocate_slice(&tmp_layout_int64_slice)),
                                                                                  SQLiteQueryLayout::allocate_span(Span_allocate_slice(&tmp_layout_text_text_int64_int64_slice)));

            l_database.assetmetadata_count_query = SQLitePreparedQuery::allocate(p_database_connection, Slice_int8_build_rawstr(AssetMetadataDatabase_Const::ASSET_METADATA_COUNT_QUERY),
                                                                                 SQLiteQueryLayout::allocate_span(Span_allocate_slice(&tmp_layout_int64_slice)),
                                                                                 SQLiteQueryLayout::allocate_span(Span_allocate_slice(&tmp_layout_int64_slice)));
            l_database.assetmetadata_select_paths_from_type = SQLitePreparedQuery::allocate(
                p_database_connection, Slice_int8_build_rawstr(AssetMetadataDatabase_Const::ASSET_METADATA_SELECT_PATHS_FROM_TYPE),
                SQLiteQueryLayout::allocate_span(Span_allocate_slice(&tmp_layout_text_slice)), SQLiteQueryLayout::allocate_span(Span_allocate_slice(&tmp_layout_text_slice)));
            l_database.assetmetadata_select_timestamps = SQLitePreparedQuery::allocate(p_database_connection, Slice_int8_build_rawstr(AssetMetadataDatabase_Const::ASSET_METADATA_SELECT_TIMESTAMPS),
                                                                                       SQLiteQueryLayout::allocate_span(Span_allocate_slice(&tmp_layout_int64_slice)),
                                                                                       SQLiteQueryLayout::allocate_span(Span_allocate_slice(&tmp_layout_int64_int64_slice)));
            return l_database;
        };

        inline void free(DatabaseConnection& p_database_connection)
        {
            this->assetmetadata_insert_query.free_with_parameterlayout(p_database_connection);
            this->assetmetadata_update_query.free_with_parameterlayout(p_database_connection);
            this->assetmetadata_select_query.free_with_parameterlayout_and_returnlayout(p_database_connection);
            this->assetmetadata_count_query.free_with_parameterlayout_and_returnlayout(p_database_connection);
            this->assetmetadata_select_paths_from_type.free_with_parameterlayout_and_returnlayout(p_database_connection);
            this->assetmetadata_select_timestamps.free_with_parameterlayout_and_returnlayout(p_database_connection);
        };

        inline int8 does_assetmetadata_exists(DatabaseConnection& p_database_connection, const hash_t p_asset_id)
        {
            SQLiteQueryBinder l_binder = SQLiteQueryBinder::build_default();
            l_binder.bind_sqlitepreparedquery(this->assetmetadata_count_query, p_database_connection);
            l_binder.bind_int64(p_asset_id, p_database_connection);

            uimax l_found = 0;
            SQLiteResultSet l_asset_rs = SQLiteResultSet::build_from_prepared_query(this->assetmetadata_count_query);
            SQliteQueryExecution::execute_sync(p_database_connection, this->assetmetadata_count_query.statement, [&l_found, &l_asset_rs]() {
                l_found = l_asset_rs.get_int64(0);
            });
            return l_found != 0;
        };

        inline void insert_metadata(DatabaseConnection& p_database_connection, const Slice<int8>& p_asset_path, const Slice<int8>& p_asset_type, const uimax p_modification_ts,
                                    const uimax p_insertion_ts)
        {
            hash_t l_id = HashSlice(p_asset_path);

            SQLiteQueryBinder l_binder = SQLiteQueryBinder::build_default();
            l_binder.bind_sqlitepreparedquery(this->assetmetadata_insert_query, p_database_connection);
            l_binder.bind_int64(l_id, p_database_connection);
            l_binder.bind_text(p_asset_path, p_database_connection);
            l_binder.bind_text(p_asset_type, p_database_connection);
            l_binder.bind_int64(p_modification_ts, p_database_connection);
            l_binder.bind_int64(p_insertion_ts, p_database_connection);

            SQliteQueryExecution::execute_sync(p_database_connection, l_binder.binded_statement, []() {
            });
        };

        inline void update_metadata(DatabaseConnection& p_database_connection, const Slice<int8>& p_asset_path, const Slice<int8>& p_asset_type, const uimax p_modification_ts,
                                    const uimax p_insertion_ts)
        {
            hash_t l_id = HashSlice(p_asset_path);

            SQLiteQueryBinder l_binder = SQLiteQueryBinder::build_default();
            l_binder.bind_sqlitepreparedquery(this->assetmetadata_update_query, p_database_connection);
            l_binder.bind_text(p_asset_path, p_database_connection);
            l_binder.bind_text(p_asset_type, p_database_connection);
            l_binder.bind_int64(p_modification_ts, p_database_connection);
            l_binder.bind_int64(p_insertion_ts, p_database_connection);
            l_binder.bind_int64(l_id, p_database_connection);

            SQliteQueryExecution::execute_sync(p_database_connection, l_binder.binded_statement, []() {
            });
        };

        inline void insert_or_update_metadata(DatabaseConnection& p_database_connection, const Slice<int8>& p_asset_path, const Slice<int8>& p_asset_type, const uimax p_modification_ts,
                                              const uimax p_insertion_ts)
        {
            hash_t l_id = HashSlice(p_asset_path);
            if (this->does_assetmetadata_exists(p_database_connection, l_id))
            {
                this->update_metadata(p_database_connection, p_asset_path, p_asset_type, p_modification_ts, p_insertion_ts);
            }
            else
            {
                this->insert_metadata(p_database_connection, p_asset_path, p_asset_type, p_modification_ts, p_insertion_ts);
            }
        };

        // id integer PRIMARY KEY, path text, type text, file_modification_ts integer, insert_ts integer

        struct MetadataTS
        {
            uimax file_modification_ts;
            uimax insert_ts;
        };

        inline MetadataTS get_timestamps(DatabaseConnection& p_database_connection, const Slice<int8>& p_asset_path)
        {
            hash_t l_id = HashSlice(p_asset_path);

            SQLiteQueryBinder l_binder = SQLiteQueryBinder::build_default();
            l_binder.bind_sqlitepreparedquery(this->assetmetadata_select_timestamps, p_database_connection);
            l_binder.bind_int64(l_id, p_database_connection);

            MetadataTS l_return;
            SQliteQueryExecution::execute_sync(p_database_connection, l_binder.binded_statement, [&]() {
                SQLiteResultSet l_rs = SQLiteResultSet::build_from_prepared_query(this->assetmetadata_select_timestamps);
                l_return.file_modification_ts = l_rs.get_int64(0);
                l_return.insert_ts = l_rs.get_int64(1);
            });

            return l_return;
        };

        struct Paths
        {
            Vector<Span<int8>> data;
            inline static Paths allocate_default()
            {
                return Paths{Vector_allocate<Span<int8>>(0)};
            };

            inline void free()
            {
                for (loop(i, 0, this->data.Size))
                {
                    Span_free(&this->data.get(i));
                }
                this->data.free();
            };
        };

        inline Paths get_all_path_from_type(DatabaseConnection& p_database_connection, const Slice<int8>& p_asset_type)
        {
            Paths l_return = Paths::allocate_default();

            SQLiteQueryBinder l_binder = SQLiteQueryBinder::build_default();
            l_binder.bind_sqlitepreparedquery(this->assetmetadata_select_paths_from_type, p_database_connection);
            l_binder.bind_text(p_asset_type, p_database_connection);

            SQLiteResultSet rs = SQLiteResultSet::build_from_prepared_query(this->assetmetadata_select_paths_from_type);
            SQliteQueryExecution::execute_sync(p_database_connection, this->assetmetadata_select_paths_from_type.statement, [&]() {
                l_return.data.push_back_element(rs.get_text(0));
            });

            return l_return;
        };

        struct AssetMetadata
        {
            Span<int8> path;
            Span<int8> type;

            inline void free()
            {
                Span_free(&this->path);
                Span_free(&this->type);
            };
        };

        inline AssetMetadata get_from_id(DatabaseConnection& p_database_connection, const hash_t p_id)
        {
            AssetMetadata l_return;

            SQLiteQueryBinder l_binder = SQLiteQueryBinder::build_default();
            l_binder.bind_sqlitepreparedquery(this->assetmetadata_select_query, p_database_connection);
            l_binder.bind_int64(p_id, p_database_connection);

            SQLiteResultSet rs = SQLiteResultSet::build_from_prepared_query(this->assetmetadata_select_query);
            SQliteQueryExecution::execute_sync(p_database_connection, this->assetmetadata_select_query.statement, [&]() {
                l_return.path = rs.get_text(0);
                l_return.type = rs.get_text(1);
            });

            return l_return;
        };
    };