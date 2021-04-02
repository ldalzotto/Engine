#pragma once

#include "Common2/common2.hpp"

namespace AssetDatabase_Const
{

static const int8* DB_ASSET_TABLE_INITIALIZATION = MULTILINE(create table if not exists asset(id integer PRIMARY KEY, value blob););
static const int8* DB_ASSET_RESSOURCE_TABLE_INITIALIZATION = MULTILINE(create table if not exists asset_dependencies(id integer PRIMARY KEY, dependencies blob););

    static const int8* ASSET_BLOB_SELECT_QUERY = MULTILINE(
			select value from asset where asset.id = ?;
	);

    static const int8* ASSET_COUNT_SELECT_QUERY = MULTILINE(
        select count(*) from asset where asset.id = ?;
    );

    static const int8* ASSET_BLOB_INSERT_QUERY = MULTILINE(
			insert into asset(id, value) values( ?, ?);
	);

    static const int8* ASSET_BLOB_UPDATE_QUERY = MULTILINE(
			update asset set value = ? where asset.id = ?;
	);

    static const int8* ASSET_DEPENDENCIES_COUNT_SELECT_QUERY = MULTILINE(select count(*) from asset_dependencies where asset_dependencies.id = ?;);

    static const int8* ASSET_DEPENDENCIES_BLOB_SELECT_QUERY = MULTILINE(
        select dependencies from asset_dependencies where asset_dependencies.id = ?;
    );

    static const int8* ASSET_DEPENDENCIES_BLOB_INSERT_QUERY = MULTILINE(insert into asset_dependencies(id, dependencies) values( ?, ?););
    static const int8* ASSET_DEPENDENCIES_BLOB_UPDATE_QUERY = MULTILINE(update asset_dependencies set dependencies = ? where asset_dependencies.id = ?;);

    } // namespace AssetDatabase_Const

    /*
        All queries are sync for now
     */
    struct AssetDatabase
    {
        SQLitePreparedQuery asset_count_query;
        SQLitePreparedQuery asset_blob_select_query;
        SQLitePreparedQuery asset_insert_query;
        SQLitePreparedQuery asset_update_query;

        SQLitePreparedQuery asset_dependencies_count_query;
        SQLitePreparedQuery asset_dependencies_blob_select_query;
        SQLitePreparedQuery asset_dependencies_insert_query;
        SQLitePreparedQuery asset_dependencies_update_query;

        inline static void initialize_database(DatabaseConnection& p_database_connection){
            {SQLiteQuery l_query = SQLiteQuery::allocate(p_database_connection, Slice_int8_build_rawstr(AssetDatabase_Const::DB_ASSET_TABLE_INITIALIZATION));
        SQliteQueryExecution::execute_sync(p_database_connection, l_query.statement, []() {
        });
        l_query.free(p_database_connection);
    }
    {
        SQLiteQuery l_query = SQLiteQuery::allocate(p_database_connection, Slice_int8_build_rawstr(AssetDatabase_Const::DB_ASSET_RESSOURCE_TABLE_INITIALIZATION));
        SQliteQueryExecution::execute_sync(p_database_connection, l_query.statement, []() {
        });
        l_query.free(p_database_connection);
    }
    }
    ;

    inline static AssetDatabase allocate(DatabaseConnection& p_database_connection)
    {
        AssetDatabase l_asset_database;

        SliceN<SQLiteQueryPrimitiveTypes, 1> tmp_layout_int64{SQLiteQueryPrimitiveTypes::INT64};
        SliceN<SQLiteQueryPrimitiveTypes, 1> tmp_layout_blob{SQLiteQueryPrimitiveTypes::BLOB};
        SliceN<SQLiteQueryPrimitiveTypes, 2> tmp_layout_int64_blob{SQLiteQueryPrimitiveTypes::INT64, SQLiteQueryPrimitiveTypes::BLOB};
        SliceN<SQLiteQueryPrimitiveTypes, 2> tmp_layout_blob_int64{SQLiteQueryPrimitiveTypes::BLOB, SQLiteQueryPrimitiveTypes::INT64};

        Slice<SQLiteQueryPrimitiveTypes> tmp_layout_int64_slice = slice_from_slicen(&tmp_layout_int64);
        Slice<SQLiteQueryPrimitiveTypes> tmp_layout_blob_slice = slice_from_slicen(&tmp_layout_blob);
        Slice<SQLiteQueryPrimitiveTypes> tmp_layout_int64_blob_slice = slice_from_slicen(&tmp_layout_int64_blob);
        Slice<SQLiteQueryPrimitiveTypes> tmp_layout_blob_int64_slice = slice_from_slicen(&tmp_layout_blob_int64);

        l_asset_database.asset_count_query = SQLitePreparedQuery::allocate(p_database_connection, Slice_int8_build_rawstr(AssetDatabase_Const::ASSET_COUNT_SELECT_QUERY),
                                                                           SQLiteQueryLayout::allocate_span(Span_allocate_slice(&tmp_layout_int64_slice)),
                                                                           SQLiteQueryLayout::allocate_span(Span_allocate_slice(&tmp_layout_int64_slice)));

        l_asset_database.asset_blob_select_query = SQLitePreparedQuery::allocate(p_database_connection, Slice_int8_build_rawstr(AssetDatabase_Const::ASSET_BLOB_SELECT_QUERY),
                                                                                 SQLiteQueryLayout::allocate_span(Span_allocate_slice(&tmp_layout_int64_slice)),
                                                                                 SQLiteQueryLayout::allocate_span(Span_allocate_slice(&tmp_layout_blob_slice)));
        l_asset_database.asset_insert_query = SQLitePreparedQuery::allocate(p_database_connection, Slice_int8_build_rawstr(AssetDatabase_Const::ASSET_BLOB_INSERT_QUERY),
                                                                            SQLiteQueryLayout::allocate_span(Span_allocate_slice(&tmp_layout_int64_blob_slice)),
                                                                            SQLiteQueryLayout::build_default());
        l_asset_database.asset_update_query = SQLitePreparedQuery::allocate(p_database_connection, Slice_int8_build_rawstr(AssetDatabase_Const::ASSET_BLOB_UPDATE_QUERY),
                                                                            SQLiteQueryLayout::allocate_span(Span_allocate_slice(&tmp_layout_blob_int64_slice)),
                                                                            SQLiteQueryLayout::build_default());

        l_asset_database.asset_dependencies_count_query =
            SQLitePreparedQuery::allocate(p_database_connection, Slice_int8_build_rawstr(AssetDatabase_Const::ASSET_DEPENDENCIES_COUNT_SELECT_QUERY),
                                          SQLiteQueryLayout::allocate_span(Span_allocate_slice(&tmp_layout_int64_slice)),
                                          SQLiteQueryLayout::allocate_span(Span_allocate_slice(&tmp_layout_int64_slice)));

        l_asset_database.asset_dependencies_blob_select_query =
            SQLitePreparedQuery::allocate(p_database_connection, Slice_int8_build_rawstr(AssetDatabase_Const::ASSET_DEPENDENCIES_BLOB_SELECT_QUERY),
                                          SQLiteQueryLayout::allocate_span(Span_allocate_slice(&tmp_layout_int64_slice)),
                                          SQLiteQueryLayout::allocate_span(Span_allocate_slice(&tmp_layout_blob_slice)));

        l_asset_database.asset_dependencies_insert_query = SQLitePreparedQuery::allocate(
            p_database_connection, Slice_int8_build_rawstr(AssetDatabase_Const::ASSET_DEPENDENCIES_BLOB_INSERT_QUERY),
            SQLiteQueryLayout::allocate_span(Span_allocate_slice(&tmp_layout_int64_blob_slice)), SQLiteQueryLayout::build_default());

        l_asset_database.asset_dependencies_update_query = SQLitePreparedQuery::allocate(
            p_database_connection, Slice_int8_build_rawstr(AssetDatabase_Const::ASSET_DEPENDENCIES_BLOB_UPDATE_QUERY),
            SQLiteQueryLayout::allocate_span(Span_allocate_slice(&tmp_layout_blob_int64_slice)), SQLiteQueryLayout::build_default());
        return l_asset_database;
    };

    inline void free(DatabaseConnection& p_database_connection)
    {
        this->asset_count_query.free_with_parameterlayout_and_returnlayout(p_database_connection);
        this->asset_update_query.free_with_parameterlayout(p_database_connection);
        this->asset_insert_query.free_with_parameterlayout(p_database_connection);
        this->asset_blob_select_query.free_with_parameterlayout_and_returnlayout(p_database_connection);
        this->asset_dependencies_count_query.free_with_parameterlayout_and_returnlayout(p_database_connection);
        this->asset_dependencies_blob_select_query.free_with_parameterlayout_and_returnlayout(p_database_connection);
        this->asset_dependencies_insert_query.free_with_parameterlayout(p_database_connection);
        this->asset_dependencies_update_query.free_with_parameterlayout(p_database_connection);
    };

    inline int8 does_asset_exists(DatabaseConnection& p_database_connection, const hash_t p_asset_id)
    {
        SQLiteQueryBinder l_binder = SQLiteQueryBinder::build_default();
        l_binder.bind_sqlitepreparedquery(this->asset_count_query, p_database_connection);
        l_binder.bind_int64(p_asset_id, p_database_connection);

        uimax l_found = 0;
        SQLiteResultSet l_asset_rs = SQLiteResultSet::build_from_prepared_query(this->asset_count_query);
        SQliteQueryExecution::execute_sync(p_database_connection, this->asset_count_query.statement, [&l_found, &l_asset_rs]() {
            l_found = l_asset_rs.get_int64(0);
        });
        return l_found != 0;
    };

    inline Span<int8> get_asset_blob(DatabaseConnection& p_database_connection, const hash_t p_asset_id)
    {
        SQLiteQueryBinder l_binder = SQLiteQueryBinder::build_default();
        l_binder.bind_sqlitepreparedquery(this->asset_blob_select_query, p_database_connection);
        l_binder.bind_int64(p_asset_id, p_database_connection);

        Span<int8> l_asset_blob = Span_build_default<int8>();
        SQLiteResultSet l_asset_rs = SQLiteResultSet::build_from_prepared_query(this->asset_blob_select_query);
        SQliteQueryExecution::execute_sync(p_database_connection, this->asset_blob_select_query.statement, [&l_asset_rs, &l_asset_blob]() {
            l_asset_blob = l_asset_rs.get_blob(0);
        });

        return l_asset_blob;
    };

    inline hash_t insert_asset_blob(DatabaseConnection& p_database_connection, const Slice<int8>& p_asset_path, const Slice<int8>& p_blob)
    {
        hash_t l_id = HashSlice(p_asset_path);

        SQLiteQueryBinder l_binder = SQLiteQueryBinder::build_default();
        l_binder.bind_sqlitepreparedquery(this->asset_insert_query, p_database_connection);
        l_binder.bind_int64(l_id, p_database_connection);
        l_binder.bind_blob(p_blob, p_database_connection);

        SQliteQueryExecution::execute_sync(p_database_connection, l_binder.binded_statement, []() {
        });

        return l_id;
    };

    inline hash_t insert_or_update_asset_blob(DatabaseConnection& p_database_connection, const Slice<int8>& p_asset_path, const Slice<int8>& p_blob)
    {
        hash_t l_id = HashSlice(p_asset_path);

        if (this->does_asset_exists(p_database_connection, l_id))
        {
            SQLiteQueryBinder l_binder = SQLiteQueryBinder::build_default();
            l_binder = SQLiteQueryBinder::build_default();
            l_binder.bind_sqlitepreparedquery(this->asset_update_query, p_database_connection);
            l_binder.bind_blob(p_blob, p_database_connection);
            l_binder.bind_int64(l_id, p_database_connection);

            SQliteQueryExecution::execute_sync(p_database_connection, l_binder.binded_statement, []() {
            });
        }
        else
        {
            SQLiteQueryBinder l_binder = SQLiteQueryBinder::build_default();
            l_binder.bind_sqlitepreparedquery(this->asset_insert_query, p_database_connection);
            l_binder.bind_int64(l_id, p_database_connection);
            l_binder.bind_blob(p_blob, p_database_connection);

            SQliteQueryExecution::execute_sync(p_database_connection, l_binder.binded_statement, []() {
            });
        }

        return l_id;
    };

    inline int8 does_asset_dependencies_exists(DatabaseConnection& p_database_connection, const hash_t p_asset_id)
    {
        SQLiteQueryBinder l_binder = SQLiteQueryBinder::build_default();
        l_binder.bind_sqlitepreparedquery(this->asset_dependencies_count_query, p_database_connection);
        l_binder.bind_int64(p_asset_id, p_database_connection);

        uimax l_count;
        SQliteQueryExecution::execute_sync(p_database_connection, l_binder.binded_statement, [&]() {
            SQLiteResultSet l_rs = SQLiteResultSet::build_from_prepared_query(this->asset_dependencies_count_query);
            l_count = l_rs.get_int64(0);
        });
        return l_count > 0;
    };

    inline Span<int8> get_asset_dependencies_blob(DatabaseConnection& p_database_connection, const hash_t p_asset_id)
    {
        SQLiteQueryBinder l_binder = SQLiteQueryBinder::build_default();
        l_binder.bind_sqlitepreparedquery(this->asset_dependencies_blob_select_query, p_database_connection);
        l_binder.bind_int64(p_asset_id, p_database_connection);

        Span<int8> l_asset_dependencies_blob = Span_build_default<int8>();
        SQLiteResultSet l_asset_rs = SQLiteResultSet::build_from_prepared_query(this->asset_dependencies_blob_select_query);
        SQliteQueryExecution::execute_sync(p_database_connection, this->asset_dependencies_blob_select_query.statement, [&l_asset_rs, &l_asset_dependencies_blob]() {
            l_asset_dependencies_blob = l_asset_rs.get_blob(0);
        });

        return l_asset_dependencies_blob;
    };

    inline void insert_asset_dependencies_blob(DatabaseConnection& p_database_connection, const Slice<int8>& p_asset_path, const Slice<int8>& p_blob)
    {
        hash_t l_id = HashSlice(p_asset_path);

        SQLiteQueryBinder l_binder = SQLiteQueryBinder::build_default();
        l_binder.bind_sqlitepreparedquery(this->asset_dependencies_insert_query, p_database_connection);
        l_binder.bind_int64(l_id, p_database_connection);
        l_binder.bind_blob(p_blob, p_database_connection);

        SQliteQueryExecution::execute_sync(p_database_connection, l_binder.binded_statement, []() {
        });
    };

    inline void update_asset_dependencies_blob(DatabaseConnection& p_database_connection, const Slice<int8>& p_asset_path, const Slice<int8>& p_blob)
    {
        hash_t l_id = HashSlice(p_asset_path);

        SQLiteQueryBinder l_binder = SQLiteQueryBinder::build_default();
        l_binder.bind_sqlitepreparedquery(this->asset_dependencies_update_query, p_database_connection);
        l_binder.bind_blob(p_blob, p_database_connection);
        l_binder.bind_int64(l_id, p_database_connection);

        SQliteQueryExecution::execute_sync(p_database_connection, l_binder.binded_statement, []() {
        });
    };

    inline void insert_or_update_asset_dependencies(DatabaseConnection& p_database_connection, const Slice<int8>& p_asset_path, const Slice<int8>& p_blob)
    {
        hash_t l_id = HashSlice(p_asset_path);
        if (this->does_asset_dependencies_exists(p_database_connection, l_id))
        {
            this->update_asset_dependencies_blob(p_database_connection, p_asset_path, p_blob);
        }
        else
        {
            this->insert_asset_dependencies_blob(p_database_connection, p_asset_path, p_blob);
        }
    };
    }
    ;