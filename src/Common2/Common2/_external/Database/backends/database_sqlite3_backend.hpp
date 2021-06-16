#pragma once

#include "sqlite3.h"

#if __DEBUG
struct SQLiteReturnCode
{
    int32 code;
};
#else
typedef int32 SQLiteReturnCode;
#endif

int32 SQLiteReturnCode_get(SQLiteReturnCode& p_code);
void SQLiteReturnCode_set(SQLiteReturnCode& p_code, int32 p_value);
SQLiteReturnCode SQLiteReturnCode_build(const int32 p_code);

#if __DEBUG
inline int32 SQLiteReturnCode_get(SQLiteReturnCode& p_code)
{
    return p_code.code;
};
inline void SQLiteReturnCode_set(SQLiteReturnCode& p_code, int32 p_value)
{
    p_code.code = p_value;
};
inline SQLiteReturnCode SQLiteReturnCode_build(const int32 p_code)
{
    return SQLiteReturnCode{p_code};
};
#else
inline int32 SQLiteReturnCode_get(SQLiteReturnCode& p_code)
{
    return p_code;
};
inline void SQLiteReturnCode_set(SQLiteReturnCode& p_code, int32 p_value)
{
    p_code = p_value;
};
inline SQLiteReturnCode SQLiteReturnCode_build(const int32 p_code)
{
    return p_code;
};
#endif

namespace DatabaseConnection_Utils
{
inline static SQLiteReturnCode handleSQLiteError_silent(SQLiteReturnCode p_return, sqlite3** p_connection)
{
#if __DEBUG
    if (SQLiteReturnCode_get(p_return) != SQLITE_OK)
    {
        String l_error_message = String::allocate_elements(slice_int8_build_rawstr("SQLITE ERROR : "));
        sqlite3_errstr(SQLiteReturnCode_get(p_return));
        if (*p_connection)
        {
            l_error_message.append(slice_int8_build_rawstr(" : "));
            l_error_message.append(slice_int8_build_rawstr(sqlite3_errmsg((sqlite3*)*p_connection)));
        }
        printf("%s\n", l_error_message.get_memory());
        l_error_message.free();
    }
#endif
    return p_return;
};

inline static SQLiteReturnCode handleSQLiteError(SQLiteReturnCode p_return, sqlite3** p_connection)
{
    handleSQLiteError_silent(p_return, p_connection);
    if (SQLiteReturnCode_get(p_return) != SQLITE_OK)
    {
        abort();
    }
    return p_return;
};

inline static SQLiteReturnCode handleStepError(SQLiteReturnCode p_step_return, sqlite3** p_connection)
{
#if __DEBUG
    int32 l_return_code = SQLiteReturnCode_get(p_step_return);
    if (l_return_code != SQLITE_BUSY && l_return_code != SQLITE_DONE && l_return_code != SQLITE_ROW)
    {
        String l_error_message = String::allocate_elements(slice_int8_build_rawstr("SQLITE ERROR : "));
        sqlite3_errstr(l_return_code);
        if (*p_connection)
        {
            l_error_message.append(slice_int8_build_rawstr(" : "));
            l_error_message.append(slice_int8_build_rawstr(sqlite3_errmsg((sqlite3*)*p_connection)));
        }
        printf("%s\n", l_error_message.get_memory());
        l_error_message.free();
        abort();
    }
#endif
    return p_step_return;
};

}; // namespace DatabaseConnection_Utils

database_connection database_connection_open_file(const Slice<int8>& p_file_path)
{
    database_connection l_connection;
    if (!p_file_path.is_null_terminated())
    {
        String l_database_path_null_terminated = String::allocate_elements(p_file_path);
        DatabaseConnection_Utils::handleSQLiteError(SQLiteReturnCode_build(sqlite3_open(l_database_path_null_terminated.get_memory(), (sqlite3**)&l_connection.ptr)), (sqlite3**)&l_connection.ptr);
        l_database_path_null_terminated.free();
    }
    else
    {
        DatabaseConnection_Utils::handleSQLiteError(SQLiteReturnCode_build(sqlite3_open(p_file_path.Begin, (sqlite3**)&l_connection.ptr)), (sqlite3**)&l_connection.ptr);
    }

    return l_connection;
};

void database_connection_close(database_connection p_database_connection)
{
    DatabaseConnection_Utils::handleSQLiteError(SQLiteReturnCode_build(sqlite3_close((sqlite3*)p_database_connection.ptr)), (sqlite3**)&p_database_connection.ptr);
};

database_statement database_statement_prepare(database_connection p_connection, const Slice<int8>& p_query)
{
    database_statement l_statement;
    DatabaseConnection_Utils::handleSQLiteError(
        SQLiteReturnCode_build(sqlite3_prepare_v3((sqlite3*)p_connection.ptr, p_query.Begin, (int32)p_query.Size, SQLITE_PREPARE_PERSISTENT, (sqlite3_stmt**)&l_statement.ptr, NULL)),
        (sqlite3**)&p_connection.ptr);
    return l_statement;
};

int8 database_statement_prepare_silent(database_connection p_connection, const Slice<int8>& p_query, database_statement* out_statement)
{
    sqlite3_stmt** l_statement = (sqlite3_stmt**)(&out_statement->ptr);
    SQLiteReturnCode l_return_code = DatabaseConnection_Utils::handleSQLiteError_silent(
        SQLiteReturnCode_build(sqlite3_prepare_v3((sqlite3*)p_connection.ptr, p_query.Begin, (int32)p_query.Size, SQLITE_PREPARE_PERSISTENT, l_statement, NULL)), (sqlite3**)&p_connection.ptr);
    if (SQLiteReturnCode_get(l_return_code) == SQLITE_OK)
    {
        return 1;
    }
    else
    {
        l_statement = NULL;
        return 0;
    }
};

void database_statement_reset(database_connection p_connection, database_statement p_statement)
{
    DatabaseConnection_Utils::handleSQLiteError(SQLiteReturnCode_build(sqlite3_reset((sqlite3_stmt*)p_statement.ptr)), (sqlite3**)&p_connection.ptr);
};

void database_statement_bind_blob(database_connection p_connection, database_statement p_statement, const int32 p_column, const Slice<int8>& p_element)
{
    DatabaseConnection_Utils::handleSQLiteError(SQLiteReturnCode_build(sqlite3_bind_blob((sqlite3_stmt*)p_statement.ptr, p_column, p_element.Begin, (int32)p_element.Size, NULL)),
                                                (sqlite3**)&p_connection.ptr);
};

void database_statement_bind_int64(database_connection p_connection, database_statement p_statement, const int32 p_column, const uimax p_element)
{
    DatabaseConnection_Utils::handleSQLiteError(SQLiteReturnCode_build(sqlite3_bind_int64((sqlite3_stmt*)p_statement.ptr, p_column, p_element)), (sqlite3**)&p_connection.ptr);
};

void database_statement_bind_text(database_connection p_connection, database_statement p_statement, const int32 p_column, const Slice<int8>& p_element)
{
    DatabaseConnection_Utils::handleSQLiteError(SQLiteReturnCode_build(sqlite3_bind_text((sqlite3_stmt*)p_statement.ptr, p_column, p_element.Begin, (int32)p_element.Size, NULL)),
                                                (sqlite3**)&p_connection.ptr);
};

uimax _database_statemennt_get_bytes(database_statement p_statement, const int32 p_column)
{
    return sqlite3_column_bytes((sqlite3_stmt*)p_statement.ptr, p_column);
};

uimax database_statement_get_int64(database_statement p_statement, const int32 p_column)
{
    return sqlite3_column_int64((sqlite3_stmt*)p_statement.ptr, p_column);
};

Slice<int8> database_statement_get_text(database_statement p_statement, const int32 p_column)
{
    Slice<int8> l_return;
    l_return.Size = _database_statemennt_get_bytes(p_statement, p_column);
    l_return.Begin = (int8*)sqlite3_column_text((sqlite3_stmt*)p_statement.ptr, p_column);
    return l_return;
};

Slice<int8> database_statement_get_blob(database_statement p_statement, const int32 p_column)
{
    Slice<int8> l_return;
    l_return.Size = _database_statemennt_get_bytes(p_statement, p_column);
    l_return.Begin = (int8*)sqlite3_column_blob((sqlite3_stmt*)p_statement.ptr, p_column);
    return l_return;
};

template <class ForeachRowFunc_t> void database_statement_execute_sync(database_connection p_connection, database_statement p_statement, const ForeachRowFunc_t& p_foreach_row)
{
    SQLiteReturnCode l_step_status = SQLiteReturnCode_build(SQLITE_BUSY);
    while (SQLiteReturnCode_get(l_step_status) == SQLITE_BUSY)
    {
        l_step_status = DatabaseConnection_Utils::handleStepError(SQLiteReturnCode_build(sqlite3_step((sqlite3_stmt*)p_statement.ptr)), (sqlite3**)&p_connection.ptr);
    }

    while (SQLiteReturnCode_get(l_step_status) == SQLITE_ROW)
    {
        p_foreach_row();

        SQLiteReturnCode_set(l_step_status, SQLITE_BUSY);
        while (SQLiteReturnCode_get(l_step_status) == SQLITE_BUSY)
        {
            l_step_status = DatabaseConnection_Utils::handleStepError(SQLiteReturnCode_build(sqlite3_step((sqlite3_stmt*)p_statement.ptr)), (sqlite3**)&p_connection.ptr);
        }
    }
};

void database_statement_finalize(database_connection p_connection, database_statement p_statement)
{
    DatabaseConnection_Utils::handleSQLiteError(SQLiteReturnCode_build(sqlite3_finalize((sqlite3_stmt*)p_statement.ptr)), (sqlite3**)&p_connection.ptr);
};
