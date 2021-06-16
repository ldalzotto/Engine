#pragma once

struct DatabaseConnection
{
    database_connection connection;

    inline static DatabaseConnection allocate(const Slice<int8>& p_databasepath)
    {
        DatabaseConnection l_connection;
        File l_database_file = File::create_or_open(p_databasepath);
        l_database_file.free();
        l_connection.connection = database_connection_open_file(p_databasepath);
#if __MEMLEAK
        push_ptr_to_tracked((int8*)l_connection.connection.ptr);
#endif
        return l_connection;
    }

    inline void free()
    {
#if __MEMLEAK
        remove_ptr_to_tracked((int8*)this->connection.ptr);
#endif
        database_connection_close(this->connection);
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
        l_return.types_slice = Slice<SQLiteQueryPrimitiveTypes>::build_default();
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
        this->types_allocated.free();
    };
};

struct SQLitePreparedQuery
{
    SQLiteQueryLayout parameter_layout;
    SQLiteQueryLayout return_layout;
    database_statement statement;

    inline static SQLitePreparedQuery allocate(DatabaseConnection& p_connection, const Slice<int8>& p_query, const SQLiteQueryLayout& p_parameter_layout, const SQLiteQueryLayout& p_return_layout)
    {
        database_statement l_statement = database_statement_prepare(p_connection.connection, p_query);

#if __MEMLEAK
        push_ptr_to_tracked(l_statement.ptr);
#endif
        return SQLitePreparedQuery{p_parameter_layout, p_return_layout, l_statement};
    };

    inline void free(DatabaseConnection& p_connection)
    {
#if __MEMLEAK
        remove_ptr_to_tracked((int8*)this->statement.ptr);
#endif
        database_statement_finalize(p_connection.connection, this->statement);
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
    database_statement statement;

    inline static SQLiteQuery allocate(DatabaseConnection& p_connection, const Slice<int8>& p_query)
    {
        database_statement l_statement = database_statement_prepare(p_connection.connection, p_query);
#if __MEMLEAK
        push_ptr_to_tracked((int8*)l_statement.ptr);
#endif
        return SQLiteQuery{l_statement};
    };

    inline static SQLiteQuery allocate_silent(DatabaseConnection& p_connection, const Slice<int8>& p_query)
    {
        database_statement l_statement;
        if (database_statement_prepare_silent(p_connection.connection, p_query, &l_statement))
        {
#if __MEMLEAK
            push_ptr_to_tracked((int8*)l_statement.ptr);
#endif
        }
        else
        {
            l_statement.ptr = NULL;
        };

        return SQLiteQuery{l_statement};
    };

    inline void free(DatabaseConnection& p_connection)
    {
#if __MEMLEAK
        remove_ptr_to_tracked((int8*)this->statement.ptr);
#endif
        database_statement_finalize(p_connection.connection, this->statement);
    };

    inline int8 is_valid()
    {
        return this->statement.ptr != NULL;
    };
};

struct SQLiteQueryBinder
{
    SQLiteQueryLayout* binded_parameter_layout;
    database_statement binded_statement;
    int8 bind_counter;

    inline static SQLiteQueryBinder build_default()
    {
        return SQLiteQueryBinder{NULL, NULL, 1};
    };

    inline void bind_sqlitepreparedquery(SQLitePreparedQuery& p_prepared_query, DatabaseConnection& p_connection)
    {
        database_statement_reset(p_connection.connection, p_prepared_query.statement);
        this->binded_statement = p_prepared_query.statement;
        this->binded_parameter_layout = &p_prepared_query.parameter_layout;
    };

    inline void bind_int64(const int64 p_value, DatabaseConnection& p_connection)
    {
#if __DEBUG
        assert_true(this->binded_parameter_layout != NULL);
        assert_true(this->binded_parameter_layout->types_slice.get(this->bind_counter - 1) == SQLiteQueryPrimitiveTypes::INT64);
#endif
        database_statement_bind_int64(p_connection.connection, this->binded_statement, this->bind_counter, p_value);
        this->bind_counter += 1;
    };

    inline void bind_text(const Slice<int8>& p_text, DatabaseConnection& p_connection)
    {
#if __DEBUG
        assert_true(this->binded_parameter_layout != NULL);
        assert_true(this->binded_parameter_layout->types_slice.get(this->bind_counter - 1) == SQLiteQueryPrimitiveTypes::TEXT);
#endif

        database_statement_bind_text(p_connection.connection, this->binded_statement, this->bind_counter, p_text);
        this->bind_counter += 1;
    };

    inline void bind_blob(const Slice<int8>& p_blob, DatabaseConnection& p_connection)
    {
#if __DEBUG
        assert_true(this->binded_parameter_layout != NULL);
        assert_true(this->binded_parameter_layout->types_slice.get(this->bind_counter - 1) == SQLiteQueryPrimitiveTypes::BLOB);
#endif
        database_statement_bind_blob(p_connection.connection, this->binded_statement, this->bind_counter, p_blob);
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
    database_statement binded_statement;

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
        assert_true(this->binded_return_layout->types_slice.get(p_index) == SQLiteQueryPrimitiveTypes::INT64);
#endif
        return database_statement_get_int64(this->binded_statement, p_index);
    };

    inline Span<int8> get_text(const int8 p_index)
    {
#if __DEBUG
        assert_true(this->binded_return_layout != NULL);
        assert_true(this->binded_return_layout->types_slice.get(p_index) == SQLiteQueryPrimitiveTypes::TEXT);
#endif
        Slice<int8> l_text = database_statement_get_text(this->binded_statement, p_index);
        return Span<int8>::allocate_slice(l_text);
    };

    inline Span<int8> get_blob(const int8 p_index)
    {
#if __DEBUG
        assert_true(this->binded_return_layout != NULL);
        assert_true(this->binded_return_layout->types_slice.get(p_index) == SQLiteQueryPrimitiveTypes::BLOB);
#endif

        return Span<int8>::allocate_slice(database_statement_get_blob(this->binded_statement, p_index));
    };
};

struct SQliteQueryExecution
{
    template <class ForeachRowFunc_t> inline static void execute_sync(DatabaseConnection& p_connection, database_statement p_statement, const ForeachRowFunc_t& p_foreach_row)
    {
        database_statement_execute_sync(p_connection.connection, p_statement, p_foreach_row);
    };
};

inline int8 DatabaseConnection_is_valid_silent(DatabaseConnection& p_database_connection)
{
    const int8* l_query = MULTILINE(drop table if exists __validation_check; create table __validation_check(c1 interger primary key); drop table __validation_check;);
    SQLiteQuery l_query_sqlite = SQLiteQuery::allocate_silent(p_database_connection, slice_int8_build_rawstr(l_query));
    int8 l_return = l_query_sqlite.is_valid();
    if (l_return)
    {
        l_query_sqlite.free(p_database_connection);
    }
    return l_return;
};