#pragma once

#include <sequelite/types.hpp>
#include <sequelite/sequelite.hpp>
#include <sqlite3.h>


namespace sequelite {

    
// Complete Definition of PIMPL struct
struct Database::Impl {
    sqlite3* db = nullptr;

    ~Impl() {
        if (db) {
            sqlite3_close(db);
            db = nullptr;
        }
    }
};

// Complete Definition of PIMPL struct
struct Statement::Impl {
    sqlite3_stmt* stmt = nullptr;
    sqlite3* db = nullptr;

    ~Impl() {
        if (stmt) {
            sqlite3_finalize(stmt);
            stmt = nullptr;
        }
    }
};


}; // namespace sequelite