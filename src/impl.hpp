#pragma once

#include <sequelite/types.hpp>
#include <sequelite/sequelite.hpp>
#include <sqlite3.h>


namespace sequelite {

    
// Complete Definition of PIMPL struct
struct Database::Impl {
    sqlite3* db = nullptr;

    ~Impl() {
        // NOTE: Removed the `if (db) {}` guard before calling `sqlite3_close(sqlite3*)` since
        //       SQLite 3 C API Officially States:
        //       EVIDENCE-OF: R-63257-11740 Calling sqlite3_close() or sqlite3_close_v2()
        //       with a NULL pointer argument is a harmless no-op.
        //       This will improve code coverage.
        sqlite3_close(db);
        db = nullptr;
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