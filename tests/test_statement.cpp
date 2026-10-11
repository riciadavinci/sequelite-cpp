#include <gtest/gtest.h>
#include <sequelite/types.hpp>
#include <sequelite/sequelite.hpp>

#include <string>
#include <optional>
#include <cstdio>

namespace sequelite::testing {

// ====================================
// === Tests for Statement Creation ===

// Fixture for Standard Statement Tests
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
        EXPECT_TRUE(result.ok());
    }

    std::optional<Database> db;
};

using StatementCreationTest = StatementTest;

TEST_F(StatementCreationTest, ValidStatementPreparationSuccess) {
    auto stmt_opt = Statement::create(*db, "SELECT * FROM test WHERE id = ?;");
    
    EXPECT_TRUE(stmt_opt);
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




}; // namespace sequelite::testing