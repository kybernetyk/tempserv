//
//  CelSQL.h
//  CelestialSQL
//
//  Created by kyb on 22/09/14.
//  Copyright (c) 2014 Celestial Machines. All rights reserved.
//

#pragma once
#include <sqlite3.h>
#include <vector>
#include <map>
#include "Types.h"

namespace viz {
    namespace sql {
        const int kSQLErrorParameterBind = 73483;
        const int kSQLTypeError = 84932;
        const int kSQLErrorUnknownColumnName = 42942;
        
        class db;
        
        class Row {
            friend db;
            
        public:
            viz::Result<int64_t> getInteger(const int idx) const;
            viz::Result<int64_t> getInteger(const std::string &col) const;
            
            viz::Result<std::string> getText(const int idx) const;
            viz::Result<std::string> getText(const std::string &col) const;
            
            viz::Result<double> getDouble(const int idx) const;
            viz::Result<double> getDouble(const std::string &col) const;
            
            viz::Result<int> getSQLType(const int idx) const;
            viz::Result<int> getSQLType(const std::string &col) const;
            
            void print() const {
                for (auto cn : m_columnIndexByName) {
                    auto colname = cn.first;
                    auto colidx = cn.second;
                    auto colval = m_columns.at(colidx);
                    if (colval.SQLType == SQLITE_INTEGER) {
                        printf("%s => %lli\n", colname.c_str(), colval.intVal);
                    }
                    if (colval.SQLType == SQLITE_FLOAT) {
                        printf("%s => %f\n", colname.c_str(), colval.doubleVal);
                    }
                    if (colval.SQLType == SQLITE_TEXT) {
                        printf("%s => %s\n", colname.c_str(), colval.textVal.c_str());
                    }
                    if (colval.SQLType == SQLITE_NULL) {
                        printf("%s => <NULL>\n", colname.c_str());
                    }
                }
            }
            
        private:
            struct AnyType {
                int64_t intVal;
                std::string textVal;
                double doubleVal;
                
                bool isNull;
                int SQLType;
            };
            
            std::map<std::string, int> m_columnIndexByName;
            std::map<int, AnyType> m_columns;
        };
        
        class Result {
            friend db;
            
        public:
            int64_t rowCount() const;
            int64_t columnCount() const;
            
            const std::vector<Row> &rows() const;
            const std::vector<std::string> &columns() const;
            
            viz::Result<std::string> columnName(int index) const;
            viz::Result<int> columnIndex(std::string columnName) const;
            
        private:
            std::map<std::string, int> m_columnIndexByName;
            std::vector<Row> m_rows;
            std::vector<std::string> m_columns;
        };
        
        //a RAII wrapper for sqlite3_stmt
        class Statement {
        public:
            Statement() {
                m_stmt = nullptr;
            }
            Statement(sqlite3_stmt *stmt) {
                m_stmt = stmt;
            }
            ~Statement() {
                if (m_stmt) {
//                    printf("finalize\n");
                    sqlite3_finalize(m_stmt);
                    m_stmt = nullptr;
                }
            }
            
            //no copy constructor, only move
            Statement(const Statement &src) = delete;
            Statement(Statement &&src) {
                m_stmt = src.m_stmt;
                src.m_stmt = nullptr;   //prevent src from finalizing m_stmt in its destructor
            }

            void reset() {
                sqlite3_reset(m_stmt);
            }
            
            sqlite3_stmt *stmt() {
                return m_stmt;
            }

            //TODO: fuck with assignment operator
            void transferOwnershipTo(Statement &other) {
                other.m_stmt = m_stmt;
                m_stmt = nullptr;
            }
            
        private:
            sqlite3_stmt *m_stmt;
        };
        
        
        class db {
        public:
            db();
            ~db();
            
            status initWithPath(const Path path, bool create);
            void close();
            
            status begin();
            status commit();
            
            status importDump(const Path path);
            
            viz::Result<Statement>prepare(const std::string &query) const;
            status bindInteger(Statement &stmt, const std::string &paramName, const int64_t value);
            status bindText(Statement &stmt, const std::string &paramName, const std::string value);
            status bindDouble(Statement &stmt, const std::string &paramName, const double value);
            status bindNull(Statement &stmt, const std::string &paramName);

            status execute(Statement &stmt);
            status execute(const std::string &query);
            
            viz::Result<int64_t> lastInsertedRowID() const;
            
            //all rows will be loaded into memory - so be wise what you query for!
            viz::Result<Result> query(const std::string &query) const;
            
        private:
            sqlite3 *m_database;
        };
        
    }
}