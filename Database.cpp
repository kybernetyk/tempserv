#include "Database.h"
#include "CelSQL.h"
#include <ctime>

namespace db {
    status addEntry(double temperature) {
        sql::db db;
        auto res = db.initWithPath("temp.db", false);
        if (!res) {
            return res.error();
        }
        
        std::time_t now;
        std::time(&now);

        std::string qry = "insert into data (timestamp, temp) VALUES (:timestamp, :temp);";
        auto stmt = db.prepare(qry);
        if (!stmt) {
            return stmt.error();
        }
        auto r = db.bindInteger(stmt.value(), ":timestamp",  now);
        if (!r) {
            return r.error();
        }
        r = db.bindDouble(stmt.value(), ":temp", temperature);
        if (!r) {
            return r.error();
        }
        r = db.execute(stmt.value());
        if (!r) {
            return r.error();
        }
        db.close();
        
        return true;
    }
}