#include "impl.hpp"

namespace sequelite {

// Constructor, Destructor & Move Semantics
Database::Database(std::unique_ptr<Impl> impl) : m_impl{std::move(impl)} {}
Database::~Database() = default;
Database::Database(Database&&) noexcept = default;
Database& Database::operator=(Database&&) noexcept = default;

Database::operator bool() const {
    return m_impl && (m_impl->db != nullptr);
}

// sequelite::Database factory:
std::optional<Database> Database::create(std::string_view filename) {
    if (filename.empty()) {
        return std::nullopt;
    }

    auto impl = std::make_unique<Impl>();
    int rc = sqlite3_open(filename.data(), &impl->db);
    if (rc != SQLITE_OK) {
        if (impl->db) {
            sqlite3_close(impl->db);
        }
        return std::nullopt;
    }

    return Database(std::move(impl));
}

DatabaseResult Database::execute(std::string_view sql_query) {
    if ((!m_impl) || (!m_impl->db)) { 
        return DatabaseResult::fail("Error: Database handle is null or Database is moved-from"); 
    }
    
    if (sql_query.empty()) {
        return DatabaseResult::fail("Error: Provided SQL query is empty");
    }

    char* error_msg {nullptr};
    int res = sqlite3_exec(m_impl->db, sql_query.data(), nullptr, nullptr, &error_msg);

    if (res != SQLITE_OK) {
        std::string error_msg_str = error_msg ? error_msg : "Unknown SQLite Error";
        sqlite3_free(error_msg);
        return DatabaseResult::fail(error_msg_str);
    }

    return DatabaseResult::success();
}

};

