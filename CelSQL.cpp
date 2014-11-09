//
//  CelSQL.cpp
//  CelestialSQL
//
//  Created by kyb on 22/09/14.
//  Copyright (c) 2014 Celestial Machines. All rights reserved.
//

#include "CelSQL.h"
#include <string>
#include <cstdio>

    namespace sql {
        
#pragma mark - row
        Result<int64_t> Row::getInteger(const int idx) const {
            if (m_columns.find(idx) == m_columns.end()) {
                return jsz::Error(kSQLErrorUnknownColumnName, __PRETTY_FUNCTION__, "Could not retrieve value for column #" + std::to_string(idx));
            }
            if (m_columns.at(idx).SQLType != SQLITE_INTEGER) {
                if (m_columns.at(idx).SQLType == SQLITE_NULL) {
                    return 0;
                }

                return jsz::Error(kSQLTypeError, __PRETTY_FUNCTION__, "Column is not SQLITE_INTEGER!");
            }
            return m_columns.at(idx).intVal;
        }
        
        Result<int64_t> Row::getInteger(const std::string &col) const {
            if (m_columnIndexByName.find(col) == m_columnIndexByName.end()) {
                return jsz::Error(kSQLErrorUnknownColumnName, __PRETTY_FUNCTION__, "Column index not found for " + col);
            }
            
            int idx = m_columnIndexByName.at(col);
            return getInteger(idx);
        }
        
        Result<std::string> Row::getText(const int idx) const {
            if (m_columns.find(idx) == m_columns.end()) {
                return jsz::Error(kSQLErrorUnknownColumnName, __PRETTY_FUNCTION__, "Could not retrieve value for column #" + std::to_string(idx));
            }
            if (m_columns.at(idx).SQLType != SQLITE_TEXT) {
                if (m_columns.at(idx).SQLType == SQLITE_NULL) {
                    return std::string("<NULL>");
                }
                return jsz::Error(kSQLTypeError, __PRETTY_FUNCTION__, "Column is not SQLITE_TEXT!");
            }
            return m_columns.at(idx).textVal;
        }
        
        Result<std::string> Row::getText(const std::string &col) const {
            if (m_columnIndexByName.find(col) == m_columnIndexByName.end()) {
                return jsz::Error(1, __PRETTY_FUNCTION__, "Column index not found for " + col);
            }
            
            int idx = m_columnIndexByName.at(col);
            return getText(idx);
        }
        
        Result<double> Row::getDouble(const int idx) const {
            if (m_columns.find(idx) == m_columns.end()) {
                return jsz::Error(kSQLErrorUnknownColumnName, __PRETTY_FUNCTION__, "Could not retrieve value for column #" + std::to_string(idx));
            }
            if (m_columns.at(idx).SQLType != SQLITE_FLOAT) {
                if (m_columns.at(idx).SQLType == SQLITE_NULL) {
                    return 0.0;
                }

                return jsz::Error(kSQLTypeError, __PRETTY_FUNCTION__, "Column is not SQLITE_FLOAT!");
            }
            return m_columns.at(idx).doubleVal;
        }
        
        Result<double> Row::getDouble(const std::string &col) const {
            if (m_columnIndexByName.find(col) == m_columnIndexByName.end()) {
                return jsz::Error(kSQLErrorUnknownColumnName, __PRETTY_FUNCTION__, "Column index not found for " + col);
            }
            
            int idx = m_columnIndexByName.at(col);
            return getDouble(idx);
        }
        
        Result<int> Row::getSQLType(const int idx) const {
            if (m_columns.find(idx) == m_columns.end()) {
                return jsz::Error(kSQLErrorUnknownColumnName, __PRETTY_FUNCTION__, "Could not retrieve value for column #" + std::to_string(idx));
            }
            return m_columns.at(idx).SQLType;
        }
        
        Result<int> Row::getSQLType(const std::string &col) const {
            if (m_columnIndexByName.find(col) == m_columnIndexByName.end()) {
                return jsz::Error(kSQLErrorUnknownColumnName, __PRETTY_FUNCTION__, "Column index not found for " + col);
            }
            
            int idx = m_columnIndexByName.at(col);
            return getSQLType(idx);
        }
        
        
#pragma mark - result
        const std::vector<Row> &QueryResult::rows() const {
            return m_rows;
        };
        
        int64_t QueryResult::rowCount() const {
            return m_rows.size();
        }
        
        int64_t QueryResult::columnCount() const {
            return columns().size();
        }
        
        const std::vector<std::string> &QueryResult::columns() const {
            return m_columns;
        }
        
        Result<std::string> QueryResult::columnName(int index) const {
            auto c = columns();
            if (index < 0 || index >= c.size()) {
                return jsz::Error(1, __PRETTY_FUNCTION__, "Index out of bounds!");
            }
            return c.at(index);
        }
        
        Result<int> QueryResult::columnIndex(std::string columnName) const {
            if (m_columnIndexByName.find(columnName) == m_columnIndexByName.end()) {
                return jsz::Error(1, __PRETTY_FUNCTION__, "Can not find column with name " + columnName);
            }
            return m_columnIndexByName.at(columnName);
        }
        
#pragma mark - db
        db::db() {
            sqlite3_config(SQLITE_CONFIG_SERIALIZED);
        }
        
        db::~db() {
            if (m_database) {
                close();
            }
        }
        
        void db::close() {
            if (m_database) {
                sqlite3_close_v2(m_database);
                m_database = nullptr;
            }
        }
        
        status db::initWithPath(const Path path, bool create) {
            int flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_FULLMUTEX;
            if (create) {
                flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX;
            }

//            int err_code = sqlite3_config(SQLITE_CONFIG_SERIALIZED);
//            if (err_code != SQLITE_OK) {
//                return jsz::Error(err_code, __PRETTY_FUNCTION__, "sqlite3_config() error!");
//            }
            
            int err_code = sqlite3_open_v2(path.to_string().c_str(),
                                           &m_database,
                                           flags,
                                           nullptr);
            if (err_code != SQLITE_OK) {
                return jsz::Error(err_code, __PRETTY_FUNCTION__, "SQLite Error (" + path.to_string() + ") : " + std::string(sqlite3_errmsg(m_database)));
            }
            
            
            return true;
        }
        
        status db::importDump(const Path path) {
            assert(m_database);

            FILE *f_in = fopen(path.to_string().c_str(), "r");
            if (!f_in) {
                return jsz::Error(1, __PRETTY_FUNCTION__, "Couldn't open file for reading: " + path.to_string());
            }
            
            char buf[255];
            std::string s;
            
            for (;;) {
                memset(buf, 0x00, 255);
                size_t r = fread(buf, 1, 254, f_in);
                if (r == 0) {
                    break;
                }
                s += std::string(buf);
            }
            fclose(f_in);
            f_in = nullptr;
            
            if (s.length() == 0) {
                return jsz::Error(2, __PRETTY_FUNCTION__, "Read 0 bytes from " + path.to_string());
            }
                        
            char *errmsg = nullptr;
            int err_code = sqlite3_exec(m_database, s.c_str(), nullptr, nullptr, &errmsg);
            if (err_code != SQLITE_OK) {
                std::string e = "<NULL>";
                if (errmsg != nullptr) {
                    e = std::string(errmsg);
                }
                auto err = jsz::Error(err_code, __PRETTY_FUNCTION__, "sqlite3_exec() Error: " + e);
                sqlite3_free(errmsg);
                return err;
            }
            
            return true;
        }
        
        Result<Statement>db::prepare(const std::string &query) const {
            assert(m_database);
            
            sqlite3_stmt *stmt;
            int err_code = sqlite3_prepare_v2(m_database,
                                              query.c_str(),
                                              (int)query.length(),
                                              &stmt,
                                              NULL);
            if (err_code != SQLITE_OK) {
                return jsz::Error(err_code, __PRETTY_FUNCTION__, "SQLite Error: " + std::string(sqlite3_errmsg(m_database)));
            }
            return Statement(stmt);
        }
        
        status db::bindInteger(Statement &stmt, const std::string &paramName, const int64_t value) {
            assert(m_database);

            int parm_idx = sqlite3_bind_parameter_index(stmt.stmt(), paramName.c_str());
            if (parm_idx == 0) {
                return jsz::Error(kSQLErrorParameterBind, __PRETTY_FUNCTION__, "Could not bind parameter: " + paramName);
            }
            
            int err_code = sqlite3_bind_int64(stmt.stmt(), parm_idx, value);
            if (err_code != SQLITE_OK) {
                return jsz::Error(kSQLErrorParameterBind, __PRETTY_FUNCTION__, "bind value SQLite Error: " + std::string(sqlite3_errmsg(m_database)));
            }
            return true;
        }
        
        status db::bindText(Statement &stmt, const std::string &paramName, const std::string value) {
            assert(m_database);

            int parm_idx = sqlite3_bind_parameter_index(stmt.stmt(), paramName.c_str());
            if (parm_idx == 0) {
                return jsz::Error(kSQLErrorParameterBind, __PRETTY_FUNCTION__, "Could not bind parameter: " + paramName);
            }
            
            int err_code = sqlite3_bind_text(stmt.stmt(), parm_idx, value.c_str(), (int)strlen(value.c_str()), SQLITE_TRANSIENT);
            if (err_code != SQLITE_OK) {
                return jsz::Error(kSQLErrorParameterBind, __PRETTY_FUNCTION__, "bind value SQLite Error: " + std::string(sqlite3_errmsg(m_database)));
            }
            
            return true;
        }
        
        status db::bindDouble(Statement &stmt, const std::string &paramName, const double value) {
            assert(m_database);

            int parm_idx = sqlite3_bind_parameter_index(stmt.stmt(), paramName.c_str());
            if (parm_idx == 0) {
                return jsz::Error(kSQLErrorParameterBind, __PRETTY_FUNCTION__, "Could not bind parameter: " + paramName);
            }
            
            int err_code = sqlite3_bind_double(stmt.stmt(), parm_idx, value);
            if (err_code != SQLITE_OK) {
                return jsz::Error(kSQLErrorParameterBind, __PRETTY_FUNCTION__, "bind value SQLite Error: " + std::string(sqlite3_errmsg(m_database)));
            }
            return true;
        }

        status db::bindNull(Statement &stmt, const std::string &paramName) {
            assert(m_database);

            int parm_idx = sqlite3_bind_parameter_index(stmt.stmt(), paramName.c_str());
            if (parm_idx == 0) {
                return jsz::Error(kSQLErrorParameterBind, __PRETTY_FUNCTION__, "Could not bind parameter: " + paramName);
            }
            
            int err_code = sqlite3_bind_null(stmt.stmt(), parm_idx);
            if (err_code != SQLITE_OK) {
                return jsz::Error(kSQLErrorParameterBind, __PRETTY_FUNCTION__, "bind value SQLite Error: " + std::string(sqlite3_errmsg(m_database)));
            }
            return true;
        }

        status db::execute(const std::string &query) {
            assert(m_database);

            Result<Statement> stmt = prepare(query);
            if (!stmt) {
                return stmt.error();
            }
            return execute(stmt.value());
        }
        
        status db::execute(Statement &stmt) {
            assert(m_database);

            int err_code = SQLITE_OK;
            for (;;) {
                err_code = sqlite3_step(stmt.stmt());
                if (err_code == SQLITE_DONE) {
                    break;
                }
                if (err_code != SQLITE_OK) {
                    return jsz::Error(err_code, __PRETTY_FUNCTION__, "Step SQLite Error: " + std::string(sqlite3_errmsg(m_database)));
                }
            }
            return true;
        }
        
        Result<QueryResult> db::query(const std::string &query) const {
            assert(m_database);

            auto stmt = prepare(query);
            if (!stmt) {
                return stmt.error();
            }
            
            QueryResult res;
            for (int i = 0; i < sqlite3_column_count(stmt.value().stmt()); i++) {
                res.m_columnIndexByName[sqlite3_column_name(stmt.value().stmt(), i)] = i;
                res.m_columns.push_back(sqlite3_column_name(stmt.value().stmt(), i));
            }
            
            Row row;
            for (;;) {
                int s = sqlite3_step(stmt.value().stmt());
                if (s == SQLITE_ROW) {
                    row.m_columnIndexByName = res.m_columnIndexByName;
                    row.m_columns.clear();
                    
                    for (int i = 0; i < sqlite3_column_count(stmt.value().stmt()); i++) {
                        int type = sqlite3_column_type(stmt.value().stmt(), i);
                        Row::AnyType val;
                        val.SQLType = type;
                        val.isNull = (type == SQLITE_NULL);
                        const unsigned char *pchar;
                        
                        //0 because let's fill the anytype with all info we can get
                        if (1) {
                            switch (type) {
                                case SQLITE_INTEGER:
                                    val.intVal = sqlite3_column_int64(stmt.value().stmt(), i);
                                    break;
                                case SQLITE_FLOAT:
                                    val.doubleVal = sqlite3_column_double(stmt.value().stmt(), i);
                                    break;
                                case SQLITE_TEXT:
                                    pchar = sqlite3_column_text(stmt.value().stmt(), i);
                                    val.textVal = std::string((const char *)pchar);
                                    break;
                                case SQLITE_NULL:
                                    val.isNull = true;
                                    val.textVal = "<NULL>";
                                    val.doubleVal = 0.0;
                                    val.intVal = 0;
                                    break;
                                    
                                default:
                                    return jsz::Error(1, __PRETTY_FUNCTION__, "Not supported column type: " + std::to_string(type));
                                    break;
                            }
                        } else {
                            val.intVal = sqlite3_column_int64(stmt.value().stmt(), i);
                            val.doubleVal = sqlite3_column_double(stmt.value().stmt(), i);
                            pchar = sqlite3_column_text(stmt.value().stmt(), i);
                            if (pchar) {
                                val.textVal = std::string((const char *)pchar);
                            } else {
                                if (type == SQLITE_NULL) {
                                    val.textVal = "<NULL>";
                                } else {
                                    val.textVal = "";
                                }
                            }
                        }
                        
                        row.m_columns[i] = val;
                    }
                    res.m_rows.push_back(row);
                } else if (s == SQLITE_DONE) {
                    break;
                } else {
                    return jsz::Error(s, __PRETTY_FUNCTION__, "bind value SQLite Error: " + std::string(sqlite3_errmsg(m_database)));
                }
            }
            return res;
        }
        
        Result<int64_t> db::lastInsertedRowID() const {
            assert(m_database);

            return sqlite3_last_insert_rowid(m_database);
        }
        
        status db::begin() {
            return execute("begin;");
        }
        status db::commit() {
            return execute("commit;");
        }
        
    }
