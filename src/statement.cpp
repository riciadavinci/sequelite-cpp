#include "impl.hpp"

namespace sequelite::internal {

namespace {
    sqlite3_destructor_type resolve_ownership(OwnershipType ownership) {
    switch (ownership) {
        case OwnershipType::Static:
            return reinterpret_cast<sqlite3_destructor_type>(SQLITE_STATIC);
        case OwnershipType::Transient:
        default:
            return reinterpret_cast<sqlite3_destructor_type>(SQLITE_TRANSIENT);
    }
} // namespace

} // namespace sequelite::internal

    
}

namespace sequelite {

// Constructor, Destructor & Move Semantics
Statement::Statement(std::unique_ptr<Impl> impl) : m_impl {std::move(impl)} {}
Statement::~Statement() = default;
Statement::Statement(Statement&&) noexcept = default;
Statement& Statement::operator=(Statement&&) noexcept = default;

Statement::operator bool() const {
    return m_impl && (m_impl->stmt != nullptr);
}


// sequelite::Statement factory:
std::optional<Statement> Statement::create(Database& db, std::string_view sql_query) {
    if (!db || sql_query.empty()) {
        return std::nullopt;
    }

    auto impl = std::make_unique<Impl>();
    impl->db = db.m_impl->db;
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(impl->db, sql_query.data(), -1, &impl->stmt, nullptr) != SQLITE_OK) {
        return std::nullopt;
    }

    return Statement(std::move(impl));
}


BindResult Statement::bind(int index, int data) {
    if (index <= 0) { return BindResult::fail("Index value must be > 0"); }
    if (!m_impl->db) { return BindResult::fail("Database handle is null"); }
    if (!m_impl->stmt) { return BindResult::fail("Statement handle is null"); }
    if (sqlite3_bind_int(m_impl->stmt, index, data) != SQLITE_OK) {
        return BindResult::fail(sqlite3_errmsg(m_impl->db));
    }
    return BindResult::success();
}

BindResult Statement::bind(int index, int64_t data) {
    if (index <= 0) { return BindResult::fail("Index value must be > 0"); }
    if (!m_impl->db) { return BindResult::fail("Database handle is null"); }
    if (!m_impl->stmt) { return BindResult::fail("Statement handle is null"); }
    if (sqlite3_bind_int64(m_impl->stmt, index, data) != SQLITE_OK) {
        return BindResult::fail(sqlite3_errmsg(m_impl->db));
    }
    return BindResult::success();
}

BindResult Statement::bind(int index, bool data) {
    if (index <= 0) { return BindResult::fail("Index value must be > 0"); }
    if (!m_impl->db) { return BindResult::fail("Database handle is null"); }
    if (!m_impl->stmt) { return BindResult::fail("Statement handle is null"); }
    int bool_data = data ? 1 : 0;
    if (sqlite3_bind_int(m_impl->stmt, index, bool_data) != SQLITE_OK) {
        return BindResult::fail(sqlite3_errmsg(m_impl->db));
    }
    return BindResult::success();
}

BindResult Statement::bind(int index, double data) {
    if (index <= 0) { return BindResult::fail("Index value must be > 0"); }
    if (!m_impl->db) { return BindResult::fail("Database handle is null"); }
    if (!m_impl->stmt) { return BindResult::fail("Statement handle is null"); }
    if (sqlite3_bind_double(m_impl->stmt, index, data) != SQLITE_OK) {
        return BindResult::fail(sqlite3_errmsg(m_impl->db));
    }
    return BindResult::success();
}

BindResult Statement::bind(int index, const std::string& data, OwnershipType ownership) {
    if (index <= 0) { return BindResult::fail("Index value must be > 0"); }
    if (!m_impl->db) { return BindResult::fail("Database handle is null"); }
    if (!m_impl->stmt) { return BindResult::fail("Statement handle is null"); }
    auto ownership_type = internal::resolve_ownership(ownership);
    if (sqlite3_bind_text(m_impl->stmt, index, data.c_str(), -1, ownership_type) != SQLITE_OK) {
        return BindResult::fail(sqlite3_errmsg(m_impl->db));
    }
    return BindResult::success();
}

BindResult Statement::bind_null(int index) {
    if (index <= 0) { return BindResult::fail("Index value must be > 0"); }
    if (!m_impl->db) { return BindResult::fail("Database handle is null"); }
    if (!m_impl->stmt) { return BindResult::fail("Statement handle is null"); }
    if (sqlite3_bind_null(m_impl->stmt, index) != SQLITE_OK) {
        return BindResult::fail(sqlite3_errmsg(m_impl->db));
    }
    return BindResult::success();
}


StepResult Statement::step() const {
    if (!m_impl->stmt) { return StepResult::fail("Statement handle is null"); }

    int res = sqlite3_step(m_impl->stmt);
    switch (res) {
        case SQLITE_ROW:
            return StepResult::success(StepState::Row, "Row Available");
        case SQLITE_DONE:
            return StepResult::success(StepState::Done, "Execution Completed");
        case SQLITE_ERROR:
        default:
            return StepResult::fail(sqlite3_errstr(res));
    }
}

StatementResult Statement::reset() const {
    if (!m_impl->stmt) { return StatementResult::fail("Statement handle is null"); }

    int res = sqlite3_reset(m_impl->stmt);
    if (res != SQLITE_OK) {
        return StatementResult::fail(sqlite3_errstr(res));
    }

    return StatementResult::success("Statement reset successfully");
}

StatementResult Statement::clear_bindings() const {
    if (!m_impl->stmt) { return StatementResult::fail("Statement handle is null"); }

    int res = sqlite3_clear_bindings(m_impl->stmt);
    if (res != SQLITE_OK) {
        return StatementResult::fail(sqlite3_errstr(res));
    }

    return StatementResult::success("Bindings cleared successfully");
}

std::optional<int> Statement::column_int(int index) const {
    if (!(m_impl->stmt) || (sqlite3_column_type(m_impl->stmt, index) == SQLITE_NULL)) {
        return std::nullopt;
    }
    return sqlite3_column_int(m_impl->stmt, index);
}

std::optional<int64_t> Statement::column_int64(int index) const {
    if (!(m_impl->stmt) || (sqlite3_column_type(m_impl->stmt, index) == SQLITE_NULL)) {
        return std::nullopt;
    }
    return static_cast<int64_t>(sqlite3_column_int64(m_impl->stmt, index));
}

std::optional<double> Statement::column_double(int index) const {
    if (!(m_impl->stmt) || (sqlite3_column_type(m_impl->stmt, index) == SQLITE_NULL)) {
        return std::nullopt;
    }
    return sqlite3_column_double(m_impl->stmt, index);
}

std::optional<bool> Statement::column_bool(int index) const {
    auto val = column_int(index);
    if (!val) { return std::nullopt; }
    return (*val) != 0;
}

std::optional<std::string> Statement::column_text(int index) const {
    if (!(m_impl->stmt) || (sqlite3_column_type(m_impl->stmt, index) == SQLITE_NULL)) {
        return std::nullopt;
    }

    const auto* text = sqlite3_column_text(m_impl->stmt, index);
    if (!text) {
        return std::nullopt;
    }

    int bytes = sqlite3_column_bytes(m_impl->stmt, index);
    return std::string(reinterpret_cast<const char*>(text), static_cast<size_t>(bytes));
}


template<>
std::optional<int> Statement::column(int index) const {
    return column_int(index);
}

template<>
std::optional<int64_t> Statement::column(int index) const {
    return column_int64(index);
}

template<>
std::optional<bool> Statement::column(int index) const {
    return column_bool(index);
}

template<>
std::optional<double> Statement::column(int index) const {
    return column_double(index);
}

template<>
std::optional<std::string> Statement::column(int index) const {
    return column_text(index);
}


};