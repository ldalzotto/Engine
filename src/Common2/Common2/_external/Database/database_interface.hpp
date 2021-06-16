#pragma once

template <class ElementType> struct Slice;

struct database_connection
{
    int8* ptr;
};

database_connection database_connection_open_file(const Slice<int8>& p_file_path);
void database_connection_close(database_connection p_database_connection);

struct database_statement
{
    int8* ptr;
};

database_statement database_statement_prepare(database_connection p_connection, const Slice<int8>& p_query);
int8 database_statement_prepare_silent(database_connection p_connection, const Slice<int8>& p_query, database_statement* out_statement);
void database_statement_reset(database_connection p_connection, database_statement p_statement);

void database_statement_bind_blob(database_connection p_connection, database_statement p_statement, const int32 p_column, const Slice<int8>& p_element);
void database_statement_bind_int64(database_connection p_connection, database_statement p_statement, const int32 p_column, const uimax p_element);
void database_statement_bind_text(database_connection p_connection, database_statement p_statement, const int32 p_column, const Slice<int8>& p_element);

uimax database_statement_get_int64(database_statement p_statement, const int32 p_column);
Slice<int8> database_statement_get_text(database_statement p_statement, const int32 p_column);
Slice<int8> database_statement_get_blob(database_statement p_statement, const int32 p_column);

template <class ForeachRowFunc_t>
void database_statement_execute_sync(database_connection p_connection, database_statement p_statement, const ForeachRowFunc_t& p_foreach_row);

void database_statement_finalize(database_connection p_connection, database_statement p_statement);
