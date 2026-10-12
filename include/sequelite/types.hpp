#pragma once

#include <optional>
#include <string>

namespace sequelite {

enum class OwnershipType {
	Transient,
	Static
};

template <typename Type>
struct Result {
	std::optional<Type> value;
	std::string message;
    bool success_flag = false;
	
	static Result success(Type value, std::string message = "Success") {
		return {std::move(value), std::move(message), true};
	}
	
	static Result fail(std::string error_message, Type error_value) {
		return {std::move(error_value), std::move(error_message), false};
	}
	
	static Result fail(std::string error_message) {
		return {std::nullopt, std::move(error_message), false};
	}
	
	explicit operator bool() const { return success_flag; }
	bool ok() const { return success_flag; }
};

template <>
struct Result<void> {
	std::string message;
    bool success_flag = false;
	
	static Result success(std::string message = "Success") {
		return {std::move(message), true};
	}
	
	static Result fail(std::string error_message) {
		return {std::move(error_message), false};
	}
	
	explicit operator bool() const { return success_flag; }
	bool ok() const { return success_flag; }
};

enum class StepState {
	Row,
	Done,
	Error
};

template <>
struct Result<StepState> {
	std::optional<StepState> value;
	std::string message;
    bool success_flag = false;
	
	static Result success(StepState value, std::string message = "Success") {
		return {value, std::move(message), true};
	}
	
	static Result fail(std::string error_message) {
		return {StepState::Error, std::move(error_message), false};
	}
	
	explicit operator bool() const { return success_flag; }
	bool ok() const { return success_flag; }

	[[nodiscard]] bool is_row() const { return value && ((*value) == StepState::Row); }
    [[nodiscard]] bool is_done() const { return value && ((*value) == StepState::Done); }
    [[nodiscard]] bool is_error() const { return value && ((*value) == StepState::Error);}
};


using StepResult = Result<StepState>;
using DatabaseResult = Result<void>;
using StatementResult = Result<void>;
using BindResult = Result<void>;

}  // namespace sequelite
