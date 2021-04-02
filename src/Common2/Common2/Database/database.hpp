#pragma once

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
        String l_error_message = String::allocate_elements(Slice_int8_build_rawstr("SQLITE ERROR : "));
        sqlite3_errstr(SQLiteReturnCode_get(p_return));
        if (*p_connection)
        {
            l_error_message.append(Slice_int8_build_rawstr(" : "));
            l_error_message.append(Slice_int8_build_rawstr(sqlite3_errmsg((sqlite3*)*p_connection)));
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
        String l_error_message = String::allocate_elements(Slice_int8_build_rawstr("SQLITE ERROR : "));
        sqlite3_errstr(l_return_code);
        if (*p_connection)
        {
            l_error_message.append(Slice_int8_build_rawstr(" : "));
            l_error_message.append(Slice_int8_build_rawstr(sqlite3_errmsg((sqlite3*)*p_connection)));
        }
        printf("%s\n", l_error_message.get_memory());
        l_error_message.free();
        abort();
    }
#endif
    return p_step_return;
};

}; // namespace DatabaseConnection_Utils

struct DatabaseConnection
{
    sqlite3* connection;

    inline static DatabaseConnection allocate(const Slice<int8>& p_databasepath)
    {
        DatabaseConnection l_connection;
        File l_database_file = File::create_or_open(p_databasepath);
        l_database_file.free();
        DatabaseConnection_Utils::handleSQLiteError(SQLiteReturnCode_build(sqlite3_open(p_databasepath.Begin, &l_connection.connection)), &l_connection.connection);

#if __MEMLEAK
        push_ptr_to_tracked((int8*)l_connection.connection);
#endif

        return l_connection;
    }

    inline void free()
    {
#if __MEMLEAK
        remove_ptr_to_tracked((int8*)this->connection);
#endif
        DatabaseConnection_Utils::handleSQLiteError(SQLiteReturnCode_build(sqlite3_close(this->connection)), &this->connection);
    }
};

enum class SQLiteQueryPrimitiveTypes
{
    UNKNOWN = 0,
    INT64 = UNKNOWN + 1,
    TEXT = INT64 + 1,
    BLOB = TEXT + 1
};

struct SQLiteQueryLayout
{
    union
    {
        Span<SQLiteQueryPrimitiveTypes> types_allocated;
        Slice<SQLiteQueryPrimitiveTypes> types_slice;
    };

    inline static SQLiteQueryLayout build_default()
    {
        SQLiteQueryLayout l_return;
        l_return.types_slice = Slice_build_default<SQLiteQueryPrimitiveTypes>();
        return l_return;
    };

    inline static SQLiteQueryLayout build_slice(const Slice<SQLiteQueryPrimitiveTypes>& p_ptimitive_types)
    {
        SQLiteQueryLayout l_return;
        l_return.types_slice = p_ptimitive_types;
        return l_return;
    };

    inline static SQLiteQueryLayout allocate_span(const Span<SQLiteQueryPrimitiveTypes>& p_ptimitive_types)
    {
        SQLiteQueryLayout l_return;
        l_return.types_allocated = p_ptimitive_types;
        return l_return;
    };

    inline void free()
    {
        Span_free(&this->types_allocated);
    };
};

struct SQLitePreparedQuery
{
    SQLiteQueryLayout parameter_layout;
    SQLiteQueryLayout return_layout;
    sqlite3_stmt* statement;

    inline static SQLitePreparedQuery allocate(DatabaseConnection& p_connection, const Slice<int8>& p_query, const SQLiteQueryLayout& p_parameter_layout, const SQLiteQueryLayout& p_return_layout)
    {
        sqlite3_stmt* l_statement;
        DatabaseConnection_Utils::handleSQLiteError(
            SQLiteReturnCode_build(sqlite3_prepare_v3(p_connection.connection, p_query.Begin, (int32)p_query.Size, SQLITE_PREPARE_PERSISTENT, &l_statement, NULL)), &p_connection.connection);
#if __MEMLEAK
        push_ptr_to_tracked((int8*)l_statement);
#endif
        return SQLitePreparedQuery{p_parameter_layout, p_return_layout, l_statement};
    };

    inline void free(DatabaseConnection& p_connection)
    {
#if __MEMLEAK
        remove_ptr_to_tracked((int8*)this->statement);
#endif
        DatabaseConnection_Utils::handleSQLiteError(SQLiteReturnCode_build(sqlite3_finalize(this->statement)), &p_connection.connection);
    };

    inline void free_with_parameterlayout(DatabaseConnection& p_connection)
    {
        this->free(p_connection);
        this->parameter_layout.free();
    };

    inline void free_with_parameterlayout_and_returnlayout(DatabaseConnection& p_connection)
    {
        this->free(p_connection);
        this->parameter_layout.free();
        this->return_layout.free();
    };
};

struct SQLiteQuery
{
    sqlite3_stmt* statement;

    inline static SQLiteQuery allocate(DatabaseConnection& p_connection, const Slice<int8>& p_query)
    {
        sqlite3_stmt* l_statement;
        DatabaseConnection_Utils::handleSQLiteError(
            SQLiteReturnCode_build(sqlite3_prepare_v3(p_connection.connection, p_query.Begin, (int32)p_query.Size, SQLITE_PREPARE_PERSISTENT, &l_statement, NULL)), &p_connection.connection);
#if __MEMLEAK
        push_ptr_to_tracked((int8*)l_statement);
#endif
        return SQLiteQuery{l_statement};
    };

    inline static SQLiteQuery allocate_silent(DatabaseConnection& p_connection, const Slice<int8>& p_query)
    {
        sqlite3_stmt* l_statement;
        SQLiteReturnCode l_return_code = DatabaseConnection_Utils::handleSQLiteError_silent(
            SQLiteReturnCode_build(sqlite3_prepare_v3(p_connection.connection, p_query.Begin, (int32)p_query.Size, SQLITE_PREPARE_PERSISTENT, &l_statement, NULL)), &p_connection.connection);
        if (SQLiteReturnCode_get(l_return_code) == SQLITE_OK)
        {
#if __MEMLEAK
            push_ptr_to_tracked((int8*)l_statement);
#endif
        }
        else
        {
            l_statement = NULL;
        }

        return SQLiteQuery{l_statement};
    };

    inline void free(DatabaseConnection& p_connection)
    {
#if __MEMLEAK
        remove_ptr_to_tracked((int8*)this->statement);
#endif
        DatabaseConnection_Utils::handleSQLiteError(SQLiteReturnCode_build(sqlite3_finalize(this->statement)), &p_connection.connection);
    };

    inline int8 is_valid()
    {
        return this->statement != NULL;
    };
};

struct SQLiteQueryBinder
{
    SQLiteQueryLayout* binded_parameter_layout;
    sqlite3_stmt* binded_statement;
    int8 bind_counter;

    inline static SQLiteQueryBinder build_default()
    {
        return SQLiteQueryBinder{NULL, NULL, 1};
    };

    inline void bind_sqlitepreparedquery(SQLitePreparedQuery& p_prepared_query, DatabaseConnection& p_connection)
    {
        DatabaseConnection_Utils::handleSQLiteError(SQLiteReturnCode_build(sqlite3_reset(p_prepared_query.statement)), &p_connection.connection);
        this->binded_statement = p_prepared_query.statement;
        this->binded_parameter_layout = &p_prepared_query.parameter_layout;
    };

    inline void bind_int64(const int64 p_value, DatabaseConnection& p_connection)
    {
#if __DEBUG
        assert_true(this->binded_parameter_layout != NULL);
        assert_true(*Slice_get(&this->binded_parameter_layout->types_slice, this->bind_counter - 1) == SQLiteQueryPrimitiveTypes::INT64);
#endif
        DatabaseConnection_Utils::handleSQLiteError(SQLiteReturnCode_build(sqlite3_bind_int64(this->binded_statement, this->bind_counter, p_value)), &p_connection.connection);
        this->bind_counter += 1;
    };

    inline void bind_text(const Slice<int8>& p_text, DatabaseConnection& p_connection)
    {
#if __DEBUG
        assert_true(this->binded_parameter_layout != NULL);
        assert_true(*Slice_get(&this->binded_parameter_layout->types_slice, this->bind_counter - 1) == SQLiteQueryPrimitiveTypes::TEXT);
#endif

        DatabaseConnection_Utils::handleSQLiteError(SQLiteReturnCode_build(sqlite3_bind_text(this->binded_statement, this->bind_counter, p_text.Begin, (int32)p_text.Size, NULL)),
                                                    &p_connection.connection);
        this->bind_counter += 1;
    };

    inline void bind_blob(const Slice<int8>& p_blob, DatabaseConnection& p_connection)
    {
#if __DEBUG
        assert_true(this->binded_parameter_layout != NULL);
        assert_true(*Slice_get(&this->binded_parameter_layout->types_slice, this->bind_counter - 1) == SQLiteQueryPrimitiveTypes::BLOB);
#endif
        DatabaseConnection_Utils::handleSQLiteError(SQLiteReturnCode_build(sqlite3_bind_blob(this->binded_statement, this->bind_counter, p_blob.Begin, (int32)p_blob.Size, NULL)),
                                                    &p_connection.connection);
        this->bind_counter += 1;
    };

    inline void clear()
    {
        this->bind_counter = 1;
    }
};

struct SQLiteResultSet
{
    SQLiteQueryLayout* binded_return_layout;
    sqlite3_stmt* binded_statement;

    inline static SQLiteResultSet build_default()
    {
        return SQLiteResultSet{NULL, NULL};
    };

    inline static SQLiteResultSet build_from_prepared_query(const SQLitePreparedQuery& p_prepared_query)
    {
        return SQLiteResultSet{(SQLiteQueryLayout*)&p_prepared_query.return_layout, p_prepared_query.statement};
    };

    inline int64 get_int64(const int8 p_index)
    {
#if __DEBUG
        assert_true(this->binded_return_layout != NULL);
        assert_true(*Slice_get(&this->binded_return_layout->types_slice, p_index) == SQLiteQueryPrimitiveTypes::INT64);
#endif
        return sqlite3_column_int64(this->binded_statement, p_index);
    };

    inline Span<int8> get_text(const int8 p_index)
    {
#if __DEBUG
        assert_true(this->binded_return_layout != NULL);
        assert_true(*Slice_get(&this->binded_return_layout->types_slice, p_index) == SQLiteQueryPrimitiveTypes::TEXT);
#endif
        uimax l_size = sqlite3_column_bytes(this->binded_statement, p_index);
        Slice<int8> l_column_as_text = Slice_build_memory_elementnb<int8>((int8*)sqlite3_column_text(this->binded_statement, p_index), l_size);
        return Span_allocate_slice<int8>(&l_column_as_text);
    };

    inline Span<int8> get_blob(const int8 p_index)
    {
#if __DEBUG
        assert_true(this->binded_return_layout != NULL);
        assert_true(*Slice_get(&this->binded_return_layout->types_slice, p_index) == SQLiteQueryPrimitiveTypes::BLOB);
#endif
        uimax l_size = sqlite3_column_bytes(this->binded_statement, p_index);
        Slice<int8> l_column_blob = Slice_build_memory_elementnb<int8>((int8*)sqlite3_column_blob(this->binded_statement, p_index), l_size);
        return Span_allocate_slice<int8>(&l_column_blob);
    };
};

struct SQliteQueryExecution
{
    template <class ForeachRowFunc_t> inline static void execute_sync(DatabaseConnection& p_connection, sqlite3_stmt* p_statement, const ForeachRowFunc_t& p_foreach_row)
    {
        SQLiteReturnCode l_step_status = SQLiteReturnCode_build(SQLITE_BUSY);
        while (SQLiteReturnCode_get(l_step_status) == SQLITE_BUSY)
        {
            l_step_status = DatabaseConnection_Utils::handleStepError(SQLiteReturnCode_build(sqlite3_step(p_statement)), &p_connection.connection);
        }

        while (SQLiteReturnCode_get(l_step_status) == SQLITE_ROW)
        {
            p_foreach_row();

            SQLiteReturnCode_set(l_step_status, SQLITE_BUSY);
            while (SQLiteReturnCode_get(l_step_status) == SQLITE_BUSY)
            {
                l_step_status = DatabaseConnection_Utils::handleStepError(SQLiteReturnCode_build(sqlite3_step(p_statement)), &p_connection.connection);
            }
        }
    };
};

inline int8 DatabaseConnection_is_valid_silent(DatabaseConnection& p_database_connection)
{
    const int8* l_query = MULTILINE(drop table if exists __validation_check; create table __validation_check(c1 interger primary key); drop table __validation_check;);
    SQLiteQuery l_query_sqlite = SQLiteQuery::allocate_silent(p_database_connection, Slice_int8_build_rawstr(l_query));
    int8 l_return = l_query_sqlite.is_valid();
    if (l_return)
    {
        l_query_sqlite.free(p_database_connection);
    }
    return l_return;
};