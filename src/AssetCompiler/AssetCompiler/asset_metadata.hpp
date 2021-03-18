#pragma once



namespace AssetMetadataDatabase_Const
{
static const int8* DB_ASSET_METADATA_TABLE_INITIALIZATION = MULTILINE(create table if not exists asset_metadata(id integer PRIMARY KEY, path text, type text););
    static const int8* ASSET_METADATA_INSERT_QUERY = MULTILINE(insert into asset_metadata(id, path, type) values(?,?,?););
    static const int8* ASSET_METADATA_UPDATE_QUERY = MULTILINE(update asset_metadata set path = ?, type = ? where asset_metadata.id = ?;);
    }; // namespace AssetMetadataDatabase_Const

    struct AssetMetadataDatabase
    {
        SQLitePreparedQuery assetmetadata_insert_query;
        SQLitePreparedQuery assetmetadata_update_query;

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
            return l_database;
        };

        inline void free(DatabaseConnection& p_database_connection)
        {
            this->assetmetadata_insert_query.free(p_database_connection);
            this->assetmetadata_update_query.free(p_database_connection);
        };
    };