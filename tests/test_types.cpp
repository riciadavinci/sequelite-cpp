#include <gtest/gtest.h>
#include <sequelite/types.hpp>

#include <optional>
#include <string>

namespace sequelite::testing {

// ==============================
// === Tests for Result<Type> ===

TEST(ResultTest, SuccessWithUserDefinedMessage) {
    auto result = Result<int>::success(42, "Custom OK");

    EXPECT_EQ(result.value, 42);
    EXPECT_EQ(result.message, "Custom OK");
    EXPECT_TRUE(result.success_flag);

    EXPECT_TRUE(result);
    EXPECT_TRUE(result.ok());
}

TEST(ResultTest, SuccessWithDefaultMessage) {
    auto result = Result<int>::success(67);

    EXPECT_EQ(result.value, 67);
    EXPECT_EQ(result.message, "Success");
    EXPECT_TRUE(result.success_flag);

    EXPECT_TRUE(result);
    EXPECT_TRUE(result.ok());
}

TEST(ResultTest, FailureWithErrorValue) {
    auto result = Result<int>::fail("Failure", -1);

    EXPECT_EQ(result.value, -1);
    EXPECT_EQ(result.message, "Failure");
    EXPECT_FALSE(result.success_flag);

    EXPECT_FALSE(result);
    EXPECT_FALSE(result.ok());
}

TEST(ResultTest, FailureWithNullopt) {
    auto result = Result<int>::fail("Error Message!");

    EXPECT_EQ(result.value, std::nullopt);
    EXPECT_EQ(result.message, "Error Message!");
    EXPECT_FALSE(result.success_flag);
  
    EXPECT_FALSE(result);
    EXPECT_FALSE(result.ok());
}

// ==============================
// === Tests for Result<void> ===

TEST(ResultVoidTest, SuccessWithDefaultMessage) {
    auto result = Result<void>::success();
  
    EXPECT_EQ(result.message, "Success");
    EXPECT_TRUE(result.success_flag);
  
    EXPECT_TRUE(result);
    EXPECT_TRUE(result.ok());
}

TEST(ResultVoidTest, SuccessWithUserDefinedMessage) {
    auto result = Result<void>::success("All Ok!");
  
    EXPECT_EQ(result.message, "All Ok!");
    EXPECT_TRUE(result.success_flag);
  
    EXPECT_TRUE(result);
    EXPECT_TRUE(result.ok());
}

TEST(ResultVoidTest, FailureWithErrorValue) {
    auto result = Result<void>::fail("Error Message!");
  
    EXPECT_EQ(result.message, "Error Message!");
    EXPECT_FALSE(result.success_flag);
  
    EXPECT_FALSE(result);
    EXPECT_FALSE(result.ok());
}

// ===================================
// === Tests for Result<StepState> ===

TEST(ResultStepStateTest, SuccessWithDefaultMessage) {
    auto result = Result<StepState>::success(StepState::Row);

    EXPECT_EQ(result.value, StepState::Row);
    EXPECT_EQ(result.message, "Success");
    EXPECT_TRUE(result.success_flag);

    EXPECT_TRUE(result);
    EXPECT_TRUE(result.ok());

    EXPECT_TRUE(result.is_row());
    EXPECT_FALSE(result.is_done());
    EXPECT_FALSE(result.is_error());
}

TEST(ResultStepStateTest, SuccessWithUserDefinedMessage) {
    auto result = Result<StepState>::success(StepState::Done, "It's Done!");

    EXPECT_EQ(result.value, StepState::Done);
    EXPECT_EQ(result.message, "It's Done!");
    EXPECT_TRUE(result.success_flag);

    EXPECT_TRUE(result);
    EXPECT_TRUE(result.ok());

    EXPECT_TRUE(result.is_done());
    EXPECT_FALSE(result.is_row());
    EXPECT_FALSE(result.is_error());
}

TEST(ResultStepStateTest, FailureConstructionWithUserDefinedMessage) {
    auto result = Result<StepState>::fail("Step Error!");

    EXPECT_EQ(result.value, StepState::Error);
    EXPECT_EQ(result.message, "Step Error!");
    EXPECT_FALSE(result.success_flag);

    EXPECT_FALSE(result);
    EXPECT_FALSE(result.ok());

    EXPECT_TRUE(result.is_error());
    EXPECT_FALSE(result.is_row());
    EXPECT_FALSE(result.is_done());
}

};