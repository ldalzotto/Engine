#pragma once

namespace AssetMetadataDatabase_Const
{
static const int8* DB_ASSET_METADATA_TABLE_INITIALIZATION = MULTILINE(create table if not exists asset_metadata(id integer PRIMARY KEY, path text, type text););
    static const int8* ASSET_METADATA_INSERT_QUERY = MULTILINE(insert into asset_metadata(id, path, type) values(?,?,?););
    static const int8* ASSET_METADATA_UPDATE_QUERY = MULTILINE(update asset_metadata set path = ?, type = ? where asset_metadata.id = ?;);
    static const int8* ASSET_METADATA_COUNT_QUERY = MULTILINE(select count(*) from asset_metadata where asset_metadata.id = ?;);
    static const int8* ASSET_METADATA_SELECT_QUERY = MULTILINE(select path, type from asset_metadata where asset_metadata.id = ?;);
    static const int8* ASSET_METADATA_SELECT_PATHS_FROM_TYPE = MULTILINE(select path from asset_metadata where asset_metadata.type = ?;);
    }; // namespace AssetMetadataDatabase_Const

    struct AssetMetadataDatabase
    {
        SQLitePreparedQuery assetmetadata_insert_query;
        SQLitePreparedQuery assetmetadata_update_query;
        SQLitePreparedQuery assetmetadata_select_query;
        SQLitePreparedQuery assetmetadata_count_query;
        SQLitePreparedQuery assetmetadata_select_paths_from_type;

        inline static void initialize_database(DatabaseConnection& p_database_connection)
        {
            SQLiteQuery l_query = SQLiteQuery::allocate(p_database_connection, slice_int8_build_rawstr(AssetMetadataDatabase_Const::DB_ASSET_METADATA_TABLE_INITIALIZATION));
            SQliteQueryExecution::execute_sync(p_database_connection, l_query.statement, []() {
            });
            l_query.free(p_database_connection);
        };

        inline static AssetMetadataDatabase allocate(DatabaseConnection& p_database_connection)
        {
            AssetMetadataDatabase l_database;
            l_database.assetmetadata_insert_query =
                SQLitePreparedQuery::allocate(p_database_connection, slice_int8_build_rawstr(AssetMetadataDatabase_Const::ASSET_METADATA_INSERT_QUERY),
                                              SQLiteQueryLayout::allocate_span(Span<SQLiteQueryPrimitiveTypes>::allocate_slice(
                                                  SliceN<SQLiteQueryPrimitiveTypes, 3>{SQLiteQueryPrimitiveTypes::INT64, SQLiteQueryPrimitiveTypes::TEXT, SQLiteQueryPrimitiveTypes::TEXT}.to_slice())),
                                              SQLiteQueryLayout::build_default());
            l_database.assetmetadata_update_query =
                SQLitePreparedQuery::allocate(p_database_connection, slice_int8_build_rawstr(AssetMetadataDatabase_Const::ASSET_METADATA_UPDATE_QUERY),
                                              SQLiteQueryLayout::allocate_span(Span<SQLiteQueryPrimitiveTypes>::allocate_slice(
                                                  SliceN<SQLiteQueryPrimitiveTypes, 3>{SQLiteQueryPrimitiveTypes::TEXT, SQLiteQueryPrimitiveTypes::TEXT, SQLiteQueryPrimitiveTypes::INT64}.to_slice())),
                                              SQLiteQueryLayout::build_default());
            l_database.assetmetadata_select_query = SQLitePreparedQuery::allocate(
                p_database_connection, slice_int8_build_rawstr(AssetMetadataDatabase_Const::ASSET_METADATA_SELECT_QUERY),
                SQLiteQueryLayout::allocate_span(Span<SQLiteQueryPrimitiveTypes>::allocate_slice(SliceN<SQLiteQueryPrimitiveTypes, 1>{SQLiteQueryPrimitiveTypes::INT64}.to_slice())),
                SQLiteQueryLayout::allocate_span(
                    Span<SQLiteQueryPrimitiveTypes>::allocate_slice(SliceN<SQLiteQueryPrimitiveTypes, 2>{SQLiteQueryPrimitiveTypes::TEXT, SQLiteQueryPrimitiveTypes::TEXT}.to_slice())));

            l_database.assetmetadata_count_query = SQLitePreparedQuery::allocate(
                p_database_connection, slice_int8_build_rawstr(AssetMetadataDatabase_Const::ASSET_METADATA_COUNT_QUERY),
                SQLiteQueryLayout::allocate_span(Span<SQLiteQueryPrimitiveTypes>::allocate_slice(SliceN<SQLiteQueryPrimitiveTypes, 1>{SQLiteQueryPrimitiveTypes::INT64}.to_slice())),
                SQLiteQueryLayout::allocate_span(Span<SQLiteQueryPrimitiveTypes>::allocate_slice(SliceN<SQLiteQueryPrimitiveTypes, 1>{SQLiteQueryPrimitiveTypes::INT64}.to_slice())));
            l_database.assetmetadata_select_paths_from_type = SQLitePreparedQuery::allocate(
                p_database_connection, slice_int8_build_rawstr(AssetMetadataDatabase_Const::ASSET_METADATA_SELECT_PATHS_FROM_TYPE),
                SQLiteQueryLayout::allocate_span(Span<SQLiteQueryPrimitiveTypes>::allocate_slice(SliceN<SQLiteQueryPrimitiveTypes, 1>{SQLiteQueryPrimitiveTypes::TEXT}.to_slice())),
                SQLiteQueryLayout::allocate_span(Span<SQLiteQueryPrimitiveTypes>::allocate_slice(SliceN<SQLiteQueryPrimitiveTypes, 1>{SQLiteQueryPrimitiveTypes::TEXT}.to_slice())));
            return l_database;
        };

        inline void free(DatabaseConnection& p_database_connection)
        {
            this->assetmetadata_insert_query.free_with_parameterlayout(p_database_connection);
            this->assetmetadata_update_query.free_with_parameterlayout(p_database_connection);
            this->assetmetadata_select_query.free_with_parameterlayout_and_returnlayout(p_database_connection);
            this->assetmetadata_count_query.free_with_parameterlayout_and_returnlayout(p_database_connection);
            this->assetmetadata_select_paths_from_type.free_with_parameterlayout_and_returnlayout(p_database_connection);
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

        inline void insert_metadata(DatabaseConnection& p_database_connection, const Slice<int8>& p_asset_path, const Slice<int8>& p_asset_type)
        {
            hash_t l_id = HashSlice(p_asset_path);

            SQLiteQueryBinder l_binder = SQLiteQueryBinder::build_default();
            l_binder.bind_sqlitepreparedquery(this->assetmetadata_insert_query, p_database_connection);
            l_binder.bind_int64(l_id, p_database_connection);
            l_binder.bind_text(p_asset_path, p_database_connection);
            l_binder.bind_text(p_asset_type, p_database_connection);

            SQliteQueryExecution::execute_sync(p_database_connection, l_binder.binded_statement, []() {
            });
        };

        inline void update_metadata(DatabaseConnection& p_database_connection, const Slice<int8>& p_asset_path, const Slice<int8>& p_asset_type)
        {
            hash_t l_id = HashSlice(p_asset_path);

            SQLiteQueryBinder l_binder = SQLiteQueryBinder::build_default();
            l_binder.bind_sqlitepreparedquery(this->assetmetadata_update_query, p_database_connection);
            l_binder.bind_text(p_asset_path, p_database_connection);
            l_binder.bind_text(p_asset_type, p_database_connection);
            l_binder.bind_int64(l_id, p_database_connection);

            SQliteQueryExecution::execute_sync(p_database_connection, l_binder.binded_statement, []() {
            });
        };

        inline void insert_or_update_metadata(DatabaseConnection& p_database_connection, const Slice<int8>& p_asset_path, const Slice<int8>& p_asset_type)
        {
            hash_t l_id = HashSlice(p_asset_path);
            if (this->does_assetmetadata_exists(p_database_connection, l_id))
            {
                this->update_metadata(p_database_connection, p_asset_path, p_asset_type);
            }
            else
            {
                this->insert_metadata(p_database_connection, p_asset_path, p_asset_type);
            }
        };

        struct Paths
        {
            Vector<Span<int8>> data;
            inline static Paths allocate_default()
            {
                return Paths{Vector<Span<int8>>::allocate(0)};
            };

            inline void free()
            {
                for (loop(i, 0, this->data.Size))
                {
                    this->data.get(i).free();
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
                this->path.free();
                this->type.free();
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