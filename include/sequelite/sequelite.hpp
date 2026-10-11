#pragma once

#include <memory>
#include <string_view>
#include "types.hpp"

namespace sequelite {

class Statement;

class Database {
public:
    [[nodiscard]] static std::optional<Database> create(std::string_view filename);
    ~Database();

    // Non-copyable, but movable
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;
    Database(Database&&) noexcept;
    Database& operator=(Database&&) noexcept;

    explicit operator bool() const;

    // Public methods:
    [[nodiscard]] DatabaseResult execute(std::string_view sql_query);

private:
    struct Impl;

    explicit Database(std::unique_ptr<Impl> impl);

    std::unique_ptr<Impl> m_impl;

    friend class Statement;
};


class Statement {
public:
    [[nodiscard]] static std::optional<Statement> create(Database& db, std::string_view sql_query);
    ~Statement();

    // Non-copyable, but movable
    Statement(const Statement&) = delete;
    Statement& operator=(const Statement&) = delete;
    Statement(Statement&&) noexcept;
    Statement& operator=(Statement&&) noexcept;

    explicit operator bool() const;

    // Public methods:
    [[nodiscard]] BindResult bind(int index, int data);
    [[nodiscard]] BindResult bind(int index, bool data);
    [[nodiscard]] BindResult bind(int index, double data);
    [[nodiscard]] BindResult bind(int index, const std::string& data, OwnershipType ownership = OwnershipType::Transient);
    [[nodiscard]] BindResult bind_null(int index);

    [[nodiscard]] StepResult step() const;
    [[nodiscard]] StatementResult reset() const;
    [[nodiscard]] StatementResult clear_bindings() const;

    [[nodiscard]] std::optional<int> column_int(int index) const;
    [[nodiscard]] std::optional<int64_t> column_int64(int index) const;
    [[nodiscard]] std::optional<double> column_double(int index) const;
    [[nodiscard]] std::optional<bool> column_bool(int index) const;
    [[nodiscard]] std::optional<std::string> column_text(int index) const;
    
    template <typename Type>
    [[nodiscard]] std::optional<Type> column(int index) const;

private:
    struct Impl;

    explicit Statement(std::unique_ptr<Impl> impl);

    std::unique_ptr<Impl> m_impl;
};

}; // namespace sequelite