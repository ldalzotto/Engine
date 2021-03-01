#pragma once

#include "Common2/common2.hpp"

namespace AssetDatabase_Const
{
static const int8* DB_INITIALIZATION = MULTILINE(create table if not exists asset(id integer PRIMARY KEY, path text, value blob));

    static const int8* ASSET_BLOB_SELECT_QUERY = MULTILINE(
			select value from asset where asset.id = ?;
	);

    static const int8* ASSET_COUNT_SELECT_QUERY = MULTILINE(
        select count(*) from asset where asset.id = ?;
    );

    static const int8* ASSET_BLOB_INSERT_QUERY = MULTILINE(
			insert into asset(id, path, value) values( ?, ?, ?);
	);

    static const int8* ASSET_BLOB_UPDATE_QUERY = MULTILINE(
			update asset set value = ? where asset.id = ?;
	);

    } // namespace AssetDatabase_Const

    /*
        All queries are sync for now
     */
    struct AssetDatabase
    {
        DatabaseConnection connection;

        SQLitePreparedQuery asset_count_query;
        SQLitePreparedQuery asset_blob_select_query;
        SQLitePreparedQuery asset_insert_query;
        SQLitePreparedQuery asset_update_query;

        inline static void initialize_database(const Slice<int8>& p_database_file_path)
        {
            DatabaseConnection l_connection = DatabaseConnection::allocate(p_database_file_path);
            SQLiteQuery l_query = SQLiteQuery::allocate(l_connection, slice_int8_build_rawstr(AssetDatabase_Const::DB_INITIALIZATION));
            SQliteQueryExecution::execute_sync(l_connection, l_query.statement, []() {});
            l_query.free(l_connection);
            l_connection.free();
        };

        inline static AssetDatabase allocate(const Slice<int8>& p_database_file_path)
        {
            AssetDatabase l_asset_database;
            l_asset_database.connection = DatabaseConnection::allocate(p_database_file_path);
            l_asset_database.asset_count_query = SQLitePreparedQuery::allocate(
                l_asset_database.connection, slice_int8_build_rawstr(AssetDatabase_Const::ASSET_COUNT_SELECT_QUERY),
                SQLiteQueryLayout::allocate_span(Span<SQLiteQueryPrimitiveTypes>::allocate_slicen(SliceN<SQLiteQueryPrimitiveTypes, 1>{SQLiteQueryPrimitiveTypes::INT64})),
                SQLiteQueryLayout::allocate_span(Span<SQLiteQueryPrimitiveTypes>::allocate_slicen(SliceN<SQLiteQueryPrimitiveTypes, 1>{SQLiteQueryPrimitiveTypes::INT64})));
            l_asset_database.asset_blob_select_query = SQLitePreparedQuery::allocate(
                l_asset_database.connection, slice_int8_build_rawstr(AssetDatabase_Const::ASSET_BLOB_SELECT_QUERY),
                SQLiteQueryLayout::allocate_span(Span<SQLiteQueryPrimitiveTypes>::allocate_slicen(SliceN<SQLiteQueryPrimitiveTypes, 1>{SQLiteQueryPrimitiveTypes::INT64})),
                SQLiteQueryLayout::allocate_span(Span<SQLiteQueryPrimitiveTypes>::allocate_slicen(SliceN<SQLiteQueryPrimitiveTypes, 1>{SQLiteQueryPrimitiveTypes::BLOB})));
            l_asset_database.asset_insert_query = SQLitePreparedQuery::allocate(l_asset_database.connection, slice_int8_build_rawstr(AssetDatabase_Const::ASSET_BLOB_INSERT_QUERY),
                                                                                SQLiteQueryLayout::allocate_span(Span<SQLiteQueryPrimitiveTypes>::allocate_slicen(SliceN<SQLiteQueryPrimitiveTypes, 3>{
                                                                                    SQLiteQueryPrimitiveTypes::INT64, SQLiteQueryPrimitiveTypes::TEXT, SQLiteQueryPrimitiveTypes::BLOB})),
                                                                                SQLiteQueryLayout::build_default());
            l_asset_database.asset_update_query = SQLitePreparedQuery::allocate(l_asset_database.connection, slice_int8_build_rawstr(AssetDatabase_Const::ASSET_BLOB_UPDATE_QUERY),
                                                                                SQLiteQueryLayout::allocate_span(Span<SQLiteQueryPrimitiveTypes>::allocate_slicen(
                                                                                    SliceN<SQLiteQueryPrimitiveTypes, 2>{SQLiteQueryPrimitiveTypes::BLOB, SQLiteQueryPrimitiveTypes::INT64})),
                                                                                SQLiteQueryLayout::build_default());
            return l_asset_database;
        };

        inline void free()
        {
            this->asset_count_query.free_with_parameterlayout_and_returnlayout(this->connection);
            this->asset_update_query.free_with_parameterlayout(this->connection);
            this->asset_insert_query.free_with_parameterlayout(this->connection);
            this->asset_blob_select_query.free_with_parameterlayout_and_returnlayout(this->connection);
            this->connection.free();
        };

        inline int8 does_asset_exists(const hash_t p_asset_id)
        {
            SQLiteQueryBinder l_binder = SQLiteQueryBinder::build_default();
            l_binder.bind_sqlitepreparedquery(this->asset_count_query, this->connection);
            l_binder.bind_int64(p_asset_id, this->connection);

            uimax l_found = 0;
            SQLiteResultSet l_asset_rs = SQLiteResultSet::build_from_prepared_query(this->asset_count_query);
            SQliteQueryExecution::execute_sync(this->connection, this->asset_count_query.statement, [&l_found, &l_asset_rs]() { l_found = l_asset_rs.get_int64(0); });
            return l_found != 0;
        };

        inline Span<int8> get_asset_blob(const hash_t p_asset_id)
        {
            SQLiteQueryBinder l_binder = SQLiteQueryBinder::build_default();
            l_binder.bind_sqlitepreparedquery(this->asset_blob_select_query, this->connection);
            l_binder.bind_int64(p_asset_id, this->connection);

            Span<int8> l_asset_blob = Span<int8>::build_default();
            SQLiteResultSet l_asset_rs = SQLiteResultSet::build_from_prepared_query(this->asset_blob_select_query);
            SQliteQueryExecution::execute_sync(this->connection, this->asset_blob_select_query.statement, [&l_asset_rs, &l_asset_blob]() { l_asset_blob = l_asset_rs.get_blob(0); });

            return l_asset_blob;
        };

        inline hash_t insert_asset_blob(const Slice<int8>& p_asset_path, const Slice<int8>& p_blob)
        {
            hash_t l_id = HashSlice(p_asset_path);

            SQLiteQueryBinder l_binder = SQLiteQueryBinder::build_default();
            l_binder.bind_sqlitepreparedquery(this->asset_insert_query, this->connection);
            l_binder.bind_int64(l_id, this->connection);
            l_binder.bind_text(p_asset_path, this->connection);
            l_binder.bind_blob(p_blob, this->connection);

            SQliteQueryExecution::execute_sync(this->connection, l_binder.binded_statement, []() {});

            return l_id;
        };

        inline hash_t insert_or_update_asset_blob(const Slice<int8>& p_asset_path, const Slice<int8>& p_blob)
        {
            hash_t l_id = HashSlice(p_asset_path);

            if (this->does_asset_exists(l_id))
            {
                SQLiteQueryBinder l_binder = SQLiteQueryBinder::build_default();
                l_binder = SQLiteQueryBinder::build_default();
                l_binder.bind_sqlitepreparedquery(this->asset_update_query, this->connection);
                l_binder.bind_blob(p_blob, this->connection);
                l_binder.bind_int64(l_id, this->connection);

                SQliteQueryExecution::execute_sync(this->connection, l_binder.binded_statement, []() {});
            }
            else
            {
                SQLiteQueryBinder l_binder = SQLiteQueryBinder::build_default();
                l_binder.bind_sqlitepreparedquery(this->asset_insert_query, this->connection);
                l_binder.bind_int64(l_id, this->connection);
                l_binder.bind_text(p_asset_path, this->connection);
                l_binder.bind_blob(p_blob, this->connection);

                SQliteQueryExecution::execute_sync(this->connection, l_binder.binded_statement, []() {});
            }

            return l_id;
        };
    };