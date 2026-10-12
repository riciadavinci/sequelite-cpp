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


// Binding failing:

TEST_F(StatementBindingTest, IntegerBindingIndexTooLarge) {
    auto stmt = Statement::create(*db, "INSERT INTO items (val_int) VALUES (?);");
    ASSERT_TRUE(stmt);
    EXPECT_FALSE(stmt->bind(2, 42).ok());
}

TEST_F(StatementBindingTest, Int64BindingIndexTooLarge) {
    auto stmt = Statement::create(*db, "INSERT INTO items (val_int64) VALUES (?);");
    ASSERT_TRUE(stmt);
    int64_t large_value = 9223372036854775807LL;
    EXPECT_FALSE(stmt->bind(2, large_value).ok());
}

TEST_F(StatementBindingTest, BooleanBindingIndexTooLarge) {
    auto stmt = Statement::create(*db, "INSERT INTO items (val_bool) VALUES (?);");
    ASSERT_TRUE(stmt);
    EXPECT_FALSE(stmt->bind(2, true).ok());
    EXPECT_FALSE(stmt->bind(2, false).ok());
}

TEST_F(StatementBindingTest, DoubleBindingIndexTooLarge) {
    auto stmt = Statement::create(*db, "INSERT INTO items (val_double) VALUES (?);");
    ASSERT_TRUE(stmt);
    EXPECT_FALSE(stmt->bind(2, 3.14159).ok());
}

TEST_F(StatementBindingTest, TextBindingTransientIndexTooLarge) {
    auto stmt = Statement::create(*db, "INSERT INTO items (val_text) VALUES (?);");
    ASSERT_TRUE(stmt);
    EXPECT_FALSE(stmt->bind(2, std::string("hello"), sequelite::OwnershipType::Transient).ok());
}

TEST_F(StatementBindingTest, TextBindingStaticIndexTooLarge) {
    auto stmt = Statement::create(*db, "INSERT INTO items (val_text) VALUES (?);");
    ASSERT_TRUE(stmt);
    EXPECT_FALSE(stmt->bind(2, "world", sequelite::OwnershipType::Static).ok());
}

TEST_F(StatementBindingTest, NullBindingIndexTooLarge) {
    auto stmt = Statement::create(*db, "INSERT INTO items (val_text) VALUES (?);");
    ASSERT_TRUE(stmt);
    EXPECT_FALSE(stmt->bind_null(2).ok());
}


// ============================================================
// === Tests for Execution, Stepping, and Column Extraction ===

class StatementExecutionAndColumnTest : public ::testing::Test {
protected:
    void SetUp() override {
        auto db_opt = Database::create(":memory:");
        ASSERT_TRUE(db_opt);
        ASSERT_TRUE(*db_opt);
        db = std::move(*db_opt);

        auto result = db->execute(
            "CREATE TABLE users ("
            "  id INTEGER PRIMARY KEY,"
            "  age INT,"
            "  salary BIGINT,"
            "  active INT,"
            "  score REAL,"
            "  nickname TEXT"
            ");"
            "INSERT INTO users (id, age, salary, active, score, nickname) VALUES (1, 30, 5000000, 1, 98.5, 'Alice');"
            "INSERT INTO users (id, age, salary, active, score, nickname) VALUES (2, NULL, NULL, 0, NULL, NULL);"
        );
        ASSERT_TRUE(result.ok());
    }

    std::optional<Database> db;
};


// Stepping Tests:

TEST_F(StatementExecutionAndColumnTest, StepRowAvailable) {
    auto stmt = Statement::create(*db, "SELECT age, nickname FROM users WHERE id = 1;");
    ASSERT_TRUE(stmt);

    auto step_res = stmt->step();
    EXPECT_TRUE(step_res.is_row());
}

TEST_F(StatementExecutionAndColumnTest, StepExecutionDone) {
    auto stmt = Statement::create(*db, "SELECT age, nickname FROM users WHERE id = 999;");
    ASSERT_TRUE(stmt);

    auto step_res = stmt->step();
    EXPECT_TRUE(step_res.is_done());
}

TEST_F(StatementExecutionAndColumnTest, StepRuntimeErrorState) {
    auto stmt = Statement::create(*db, "INSERT INTO users (id) VALUES (1);");
    ASSERT_TRUE(stmt);

    auto step_res = stmt->step();
    EXPECT_TRUE(step_res.is_error());
}



// Reset & Clear Bindings Tests:

TEST_F(StatementExecutionAndColumnTest, StatementResetSuccess) {
    auto stmt = Statement::create(*db, "SELECT id FROM users ORDER BY id;");
    ASSERT_TRUE(stmt);

    // Step First Row:
    ASSERT_TRUE(stmt->step().is_row());
    EXPECT_EQ(stmt->column_int(0), 1);

    // Resetting statement to allow re-stepping from the beginning
    auto reset_res = stmt->reset();
    EXPECT_TRUE(reset_res.ok());

    // Re-step and verify we get the same row again:
    ASSERT_TRUE(stmt->step().is_row());
    EXPECT_EQ(stmt->column_int(0), 1);    
}


TEST_F(StatementExecutionAndColumnTest, StatementClearBindingsSuccess) {
    auto stmt = Statement::create(*db, "SELECT * FROM users WHERE id = ?;");
    ASSERT_TRUE(stmt);
    EXPECT_TRUE(stmt->bind(1, static_cast<int>(1)).ok());

    auto clear_res = stmt->clear_bindings();
    EXPECT_TRUE(clear_res.ok());
}


// Column Extraction & Template Helpers Tests

TEST_F(StatementExecutionAndColumnTest, ExplicitTypeGettersAndTemplates) {
    auto stmt = Statement::create(*db, "SELECT id, age, salary, active, score, nickname FROM users WHERE id = 1;");
    ASSERT_TRUE(stmt);

    ASSERT_TRUE(stmt->step().is_row());

    // 1. Explicit Type Getters (exact data fidelity check)
    EXPECT_EQ(stmt->column_int(0), 1);
    EXPECT_EQ(stmt->column_int(1), 30);
    EXPECT_EQ(stmt->column_int64(2), 5000000LL);
    EXPECT_EQ(stmt->column_bool(3), true);
    EXPECT_DOUBLE_EQ(*(stmt->column_double(4)), 98.5);
    EXPECT_EQ(stmt->column_text(5), "Alice");


    // 2. Template Specialization Tests (delegation check)
    EXPECT_EQ(stmt->column<int>(0), 1);
    EXPECT_EQ(stmt->column<int>(1), 30);
    EXPECT_EQ(stmt->column<int64_t>(2), 5000000LL);
    EXPECT_EQ(stmt->column<bool>(3), true);
    EXPECT_DOUBLE_EQ(*(stmt->column<double>(4)), 98.5);
    EXPECT_EQ(stmt->column<std::string>(5), "Alice");
}

TEST_F(StatementExecutionAndColumnTest, NullValueHandling) {
    auto stmt = Statement::create(*db, "SELECT age, salary, score, nickname FROM users WHERE id = 2;");
    ASSERT_TRUE(stmt);

    ASSERT_TRUE(stmt->step().is_row());

    // Verify NULL columns safely return std::nullopt instead of crashing
    EXPECT_FALSE(stmt->column_int(0).has_value());
    EXPECT_FALSE(stmt->column_int64(1).has_value());
    EXPECT_FALSE(stmt->column_double(2).has_value());
    EXPECT_FALSE(stmt->column_text(3).has_value());


    // Verify NULL columns safely return std::nullopt instead of crashing
    EXPECT_FALSE(stmt->column<int>(0).has_value());
    EXPECT_FALSE(stmt->column<int64_t>(1).has_value());
    EXPECT_FALSE(stmt->column<double>(2).has_value());
    EXPECT_FALSE(stmt->column<std::string>(3).has_value());
}

TEST_F(StatementExecutionAndColumnTest, NullStatementColumnExtractionFailure) {
    auto stmt = Statement::create(*db, "SELECT age, salary, active, score, nickname FROM users WHERE id = 1;");
    ASSERT_TRUE(stmt);
    ASSERT_TRUE(stmt->step().is_row());

    Statement moved_to_stmt(std::move(*stmt));

    EXPECT_FALSE(stmt->column_int(0).has_value());
    EXPECT_FALSE(stmt->column_int64(1).has_value());
    EXPECT_FALSE(stmt->column_bool(2).has_value());
    EXPECT_FALSE(stmt->column_double(3).has_value());
    EXPECT_FALSE(stmt->column_text(4).has_value());

    EXPECT_FALSE(stmt->column<int>(0).has_value());
    EXPECT_FALSE(stmt->column<int64_t>(1).has_value());
    EXPECT_FALSE(stmt->column<bool>(2).has_value());
    EXPECT_FALSE(stmt->column<double>(3).has_value());
    EXPECT_FALSE(stmt->column<std::string>(4).has_value());
}

}; // namespace sequelite::testing