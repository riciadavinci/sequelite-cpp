#include <gtest/gtest.h>
#include <sequelite/types.hpp>
#include <sequelite/sequelite.hpp>

#include <string>
#include <optional>
#include <cstdio>

namespace sequelite::testing {

// ============================================
// === Fixture for Standard Statement Tests ===

class StatementTest : public ::testing::Test {
protected:
    void SetUp() override {
        auto db_opt = Database::create(":memory:");
        ASSERT_TRUE(db_opt.has_value());
        ASSERT_TRUE(db_opt.value());
        db = std::move(*db_opt);

        // Pre-create 'test' table
        std::string sql_query = "CREATE TABLE test (id INT, name TEXT);";
        auto result = db->execute(sql_query);
        ASSERT_TRUE(result.ok());
    }

    std::optional<Database> db;
};

using StatementCreationTest = StatementTest;
using StatementSemanticsTest = StatementTest;


// ====================================
// === Tests for Statement Creation ===

TEST_F(StatementCreationTest, ValidStatementPreparationSuccess) {
    auto stmt_opt = Statement::create(*db, "SELECT * FROM test WHERE id = ?;");
    
    EXPECT_TRUE(stmt_opt);
    EXPECT_TRUE(stmt_opt.value());
}

TEST_F(StatementCreationTest, InvalidDatabaseGuardFailure) {
    Database moved_db = std::move(*db);

    auto stmt_opt = Statement::create(*db, "SELECT * FROM test WHERE id = ?;");
    
    EXPECT_FALSE(stmt_opt);
}

TEST_F(StatementCreationTest, EmptyQueryGuardFailure) {
    auto stmt_opt = Statement::create(*db, "");
    
    EXPECT_FALSE(stmt_opt);
}

TEST_F(StatementCreationTest, InvalidSqlQueryFailure) {
    auto stmt_opt = Statement::create(*db, "SELECT * FROM test WHER id = ?;");

    EXPECT_FALSE(stmt_opt);
}

// =====================================
// === Tests for Statement Semantics ===

TEST_F(StatementSemanticsTest, MoveConstructorSuccess) {
    auto stmt_opt = Statement::create(*db, "SELECT * FROM test WHERE id = ?;");
    ASSERT_TRUE(stmt_opt);

    Statement stmt(std::move(*stmt_opt));
    EXPECT_TRUE(stmt);
    EXPECT_FALSE(*stmt_opt);
}

TEST_F(StatementSemanticsTest, MoveAssignmentSuccess) {
    auto stmt_opt1 = Statement::create(*db, "SELECT * FROM test WHERE id = ?;");
    auto stmt_opt2 = Statement::create(*db, "SELECT * FROM test;");
    ASSERT_TRUE(stmt_opt1);
    ASSERT_TRUE(stmt_opt2);

    Statement stmt1(std::move(*stmt_opt1));
    Statement stmt2(std::move(*stmt_opt2));

    stmt2 = std::move(stmt1);
    EXPECT_TRUE(stmt2);
    EXPECT_FALSE(stmt1);
}


// ===============================================
// === Tests for Statement.bind(...) overloads ===

class StatementBindingTest : public ::testing::Test {
protected:
    void SetUp() override {
        auto db_opt = Database::create(":memory:");
        ASSERT_TRUE(db_opt.has_value());
        ASSERT_TRUE(db_opt.value());

        db = std::move(*db_opt);
        ASSERT_TRUE(*db);

        // Creating a table covering all types covered by bind() method overloads
        auto result = db->execute(
            "CREATE TABLE items ("
            "  id INTEGER PRIMARY KEY,"
            "  val_int INT,"
            "  val_int64 BIGINT,"
            "  val_bool INT,"
            "  val_double REAL,"
            "  val_text TEXT"
            ");"
        );
        ASSERT_TRUE(result.ok());
    }

    std::optional<Database> db;
};


TEST_F(StatementBindingTest, IntegerBinding) {
    auto stmt = Statement::create(*db, "INSERT INTO items (val_int) VALUES (?);");
    ASSERT_TRUE(stmt);
    EXPECT_TRUE(stmt->bind(1, 42).ok());
}

TEST_F(StatementBindingTest, Int64Binding) {
    auto stmt = Statement::create(*db, "INSERT INTO items (val_int64) VALUES (?);");
    ASSERT_TRUE(stmt);
    int64_t large_value = 9223372036854775807LL;
    EXPECT_TRUE(stmt->bind(1, large_value).ok());
}

TEST_F(StatementBindingTest, BooleanBinding) {
    auto stmt = Statement::create(*db, "INSERT INTO items (val_bool) VALUES (?);");
    ASSERT_TRUE(stmt);
    EXPECT_TRUE(stmt->bind(1, true).ok());
    EXPECT_TRUE(stmt->bind(1, false).ok());
}

TEST_F(StatementBindingTest, DoubleBinding) {
    auto stmt = Statement::create(*db, "INSERT INTO items (val_double) VALUES (?);");
    ASSERT_TRUE(stmt);
    EXPECT_TRUE(stmt->bind(1, 3.14159).ok());
}

TEST_F(StatementBindingTest, TextBindingTransient) {
    auto stmt = Statement::create(*db, "INSERT INTO items (val_text) VALUES (?);");
    ASSERT_TRUE(stmt);
    EXPECT_TRUE(stmt->bind(1, std::string("hello"), sequelite::OwnershipType::Transient).ok());
}

TEST_F(StatementBindingTest, TextBindingStatic) {
    auto stmt = Statement::create(*db, "INSERT INTO items (val_text) VALUES (?);");
    ASSERT_TRUE(stmt);
    EXPECT_TRUE(stmt->bind(1, "world", sequelite::OwnershipType::Static).ok());
}

TEST_F(StatementBindingTest, NullBinding) {
    auto stmt = Statement::create(*db, "INSERT INTO items (val_text) VALUES (?);");
    ASSERT_TRUE(stmt);
    EXPECT_TRUE(stmt->bind_null(1).ok());
}

}; // namespace sequelite::testing