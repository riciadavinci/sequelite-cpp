# sequelite-cpp

<!-- Badges & Shields -->
[![C++20](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://en.cppreference.com/w/cpp/20)
[![CMake](https://img.shields.io/badge/CMake-3.20%2B-064F8C.svg?logo=cmake)](https://cmake.org/)
[![Build Status](https://github.com/riciadavinci/sequelite-cpp/actions/workflows/ci.yaml/badge.svg)](https://github.com/riciadavinci/sequelite-cpp/actions)
[![codecov](https://codecov.io/gh/riciadavinci/sequelite-cpp/graph/badge.svg?token=O7VO610R9Q)](https://codecov.io/gh/riciadavinci/sequelite-cpp)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

## 1. Overview

## 2. Quick Start

Getting up and running with _**sequelite-cpp**_ is straightforward. The library is designed with modern _**C++20**_ principles, relying on RAII, `std::optional` for safe error handling, and clean separation between database and statement lifecycles.

### Prerequisites
- A C++20 compliant compiler (MSVC 2022+, GCC 11+, or Clang 13+)
- CMake 3.20 or higher
- SQLite3 development libraries

### Quick Example
Here is a complete example demonstrating how to initialize a database, create a prepared statement, bind parameters, and step through results using the sequelite-cpp API:

```c++
#include <sequelite/database.hpp>
#include <sequelite/statement.hpp>
#include <iostream>
#include <cstdlib>
#include <string>

struct User {
    int id;
    std::string name;    
};

int main() {
    // 1. Create or open a database connection
    auto db_opt = sequelite::Database::create("example.db");
    if (!db_opt.has_value()) {
        std::cerr << "Failed to create database connection.\n";
        return EXIT_FAILURE;
    }
    sequelite::Database db = std::move(*db_opt);

    // 2. Execute standard SQL (e.g., table creation & data insertion)
    auto exec_res = db.execute(
        "CREATE TABLE IF NOT EXISTS users (id INTEGER PRIMARY KEY, name TEXT);"
        "INSERT INTO users (id, name) VALUES (42, 'John');"
    );
    if (!exec_res.ok()) {
        std::cerr << "Failed to create table & insert data.\n";
        return EXIT_FAILURE;
    }

    // 3. Prepare a parameterized statement
    auto stmt_opt = sequelite::Statement::create(db, "SELECT * FROM users WHERE id = ?;");
    if (!stmt_opt.has_value()) {
        std::cerr << "Failed to prepare statement.\n";
        return EXIT_FAILURE;
    }
    sequelite::Statement stmt = std::move(*stmt_opt);

    // 4. Bind parameters and execute
    if (auto res = stmt.bind<int>(1, 42); !res.ok()) {
        std::cerr << "Failed to bind parameter.\n";
        return EXIT_FAILURE;
    }

    User user;

    // 5. Step through result rows
    if (auto step_res = stmt.step(); step_res.ok()) {
        // Retrieve columns (using 0-based indexing for columns)
        auto user_id_opt = stmt.column<int>(0);
        if (user_id_opt.has_value()) {
            user.id = *user_id_opt;
        }

        auto user_name_opt = stmt.column<std::string>(1);
        if (user_name_opt.has_value()) {
            user.name = *user_name_opt;
        }

        std::cout << "Successfully retrieved user: " << user.name << " (ID: " << user.id << ")\n";
    }

    return EXIT_SUCCESS;
}
```

<br>

<p align="right"><sub>[&nbsp;<a href="#sequelite-cpp">Back to Top</a> &nbsp;•&nbsp; <a href="#table-of-contents">Table of Contents</a>&nbsp;]</sub></p>


## Table of Contents
| No. | Section |
| --- | --- |
| **1.** | [Overview](#1-overview) |
| **2.** | [Quick Start](#2-quick-start) |
| **3.** | [Tech Stack](#3-tech-stack) |
| **4.** | [Project Structure](#4-project-structure) |
| **5.** | [Installation & Integration](#5-installation--integration) |
|  | **5.1** &emsp;[Manual Include Path](#51-manual-include-path) |
|  | **5.2** &emsp;[Using _CMake_'s `FetchContent`](#52-using-cmakes-fetchcontent) |
| **6.** | [Features / Design Goals](#6-features--design-goals) |
| **7.** | [API Overview](#7-api-overview) |
|  | **7.1** &emsp;[Class: `Database`](#7-api-overview) |
|  | **7.2** &emsp;[Class: `Statement`](#7-api-overview) |
| **8.** | [Error Handling](#8-error-handling) |

<br>

<p align="right"><sub>[&nbsp;<a href="#sequelite-cpp">Back to Top</a> &nbsp;•&nbsp; <a href="#table-of-contents">Table of Contents</a>&nbsp;]</sub></p>


## 3. Tech Stack

- _**Programming Language:** C++ (C++20 standard), C++ Standard Library_
- _**Testing & Code Coverage:** Google Test, Google Mock, GCov, Codecov_
- _**Linting & Sanitizers:** Clang-Tidy, ASan, UBSan, TSan_
- _**Build Tools:** CMake 3.30.5, Ninja 1.31.1_
- _**Target OS Tested on:**  Windows 10 & Ubuntu 26.04_
- _**Compilers Tested on:**_
    - _**Windows:** MSVC 2022 (v19.41.34123 x64)_
    - _**Linux:** GCC (13.2) & Clang_ 
- _**External Libs/Deps:** SQLite3 Amalgamation src *V3.54.0*_

<p align="right"><sub>[&nbsp;<a href="#sequelite-cpp">Back to Top</a> &nbsp;•&nbsp; <a href="#table-of-contents">Table of Contents</a>&nbsp;]</sub></p>

## 4. Project Structure

```text
sequelite-cpp/
├── include/
│   └── sequelite/
│       ├── types.hpp
│       └── sequelite.hpp
├── src/
│   ├── impl.hpp
│   ├── database.cpp
│   └── statement.cpp
├── tests/                     
│   ├── CMakeLists.txt
│   ├── test_types.cpp
│   ├── test_database.cpp
│   └── test_statement.cpp
├── CMakeLists.txt
├── README.md
└── LICENSE
```

<p align="right"><sub>[&nbsp;<a href="#sequelite-cpp">Back to Top</a> &nbsp;•&nbsp; <a href="#table-of-contents">Table of Contents</a>&nbsp;]</sub></p>

## 5. Installation & Integration

### Pre-requisites
Before building, ensure you have the following installed:
- _**CMake:** (v3.14 or higher)_
- _**C++20 Compiler:** (MSVC 2022, GCC 13+, or Clang 16+)_
- _**Ninja:** (Recommended build generator)_
- _**Git:** (Recommended for downloading)_

<br>

### 5.1 Manual Include Path

If you prefer not to use automated fetching utilities like `FetchContent`, _**sequelite-cpp**_ can be manually integrated into your project by downloading the source tree locally and linking it via _**CMake**_. Because the library requires compiling its implementation files alongside your own, ensure your build system includes the library's source target.

<br>

#### Step 1: Download the Source

Clone the repository or download the specific release archive directly into your project's directory structure (for example, placing it into a local `third_party/`, `libs/` or `deps/` folder):

```bash
git clone https://github.com/riciadavinci/sequelite-cpp.git third_party/sequelite-cpp
```

<br>

#### Step 2: Update Your Build Configuration

Because sequelite-cpp contains compiled source files, the most robust manual approach is to ingest it as a local subdirectory in your CMakeLists.txt. This automatically handles both header inclusion and source compilation targets.

Update your local CMakeLists.txt:
```cmake
cmake_minimum_required(VERSION 3.14)
project(my_application LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. Add the local library source directory
add_subdirectory(third_party/sequelite-cpp)

# 2. Create your application executable
add_executable(my_application main.cpp)

# 3. Link against the library target (this automatically pulls in include paths and compiled sources)
target_link_libraries(my_application PRIVATE sequelite::sequelite)
```

>Note for Raw CLI or Custom Makefile Users: If you are building manually via raw compiler commands (like g++) or traditional Makefiles rather than CMake, you will need to compile the library's source files (.cpp) alongside your main.cpp and link against SQLite3 (-lsqlite3) directly. CMake is strongly recommended to manage these dependencies seamlessly.


<br>

<p align="right"><sub>[&nbsp;<a href="#sequelite-cpp">Back to Top</a> &nbsp;•&nbsp; <a href="#table-of-contents">Table of Contents</a>&nbsp;]</sub></p>


### 5.2 Using _CMake's_ `FetchContent`

For modern CMake projects, you can integrate this library directly into your build system without downloading it manually. Add the following to your `CMakeLists.txt`:

```cmake
include(FetchContent)

FetchContent_Declare(
    sequelite_cpp
    GIT_REPOSITORY https://github.com/riciadavinci/sequelite-cpp.git
    GIT_TAG        main # Replace with a specific tag/commit hash for stability
)

FetchContent_MakeAvailable(sequelite_cpp)
```

#### Linking to your target
Once fetched, link your application or executable to the interface library target:

```cmake
target_link_libraries(your_project_target PRIVATE sequelite::sequelite)
```

<br>

<p align="right"><sub>[&nbsp;<a href="#sequelite-cpp">Back to Top</a> &nbsp;•&nbsp; <a href="#table-of-contents">Table of Contents</a>&nbsp;]</sub></p>

## 6. Features / Design Goals

- **Modern C++20:** Built using contemporary language features like `std::string_view` and `std::optional` for expressiveness and safety.
- **PIMPL Idiom (Compilation Firewall):** Encapsulates implementation details and hides _**SQLite C headers**_ from public interfaces, guaranteeing fast compilation times and clean dependency trees.
- **RAII Compliance:** Automatic resource acquisition and cleanup via strict move-only semantics.
- **Exception-Free Design:** Predictable performance using `std::optional` and explicit status codes.
- **Type Safety:** Explicit template bindings and type-checked column retrieval.

<br>

<p align="right"><sub>[&nbsp;<a href="#sequelite-cpp">Back to Top</a> &nbsp;•&nbsp; <a href="#table-of-contents">Table of Contents</a>&nbsp;]</sub></p>

## 7. API Overview

### 7.1 SqliteDatabase

```c++
namespace sequelite {

class Database {
public:
    // Static Factory Method
    static std::optional<Database> create(std::string_view filename);

    // Non-copyable, but movable
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;
    Database(Database&&) noexcept;
    Database& operator=(Database&&) noexcept;

    explicit operator bool() const;

    DatabaseResult execute(std::string_view sql_query); 
};

} // namespace sequelite
```

<br>

<p align="right"><sub>[&nbsp;<a href="#sequelite-cpp">Back to Top</a> &nbsp;•&nbsp; <a href="#table-of-contents">Table of Contents</a>&nbsp;]</sub></p>

### 7.2 SqliteStatement

```c++
namespace sequelite {

class Statement {
public:
    // Static Factory Method
    static std::optional<Statement> create(Database& db, std::string_view sql_query);

    // Non-copyable, but movable
    Statement(const Statement&) = delete;
    Statement& operator=(const Statement&) = delete;
    Statement(Statement&&) noexcept;
    Statement& operator=(Statement&&) noexcept;

    BindResult bind(int index, int data);
    BindResult bind(int index, bool data);
    BindResult bind(int index, double data);
    BindResult bind(int index, const std::string& data, OwnershipType ownership = OwnershipType::Transient);
    BindResult bind_null(int index);

    StepResult step() const;
    StatementResult reset() const;
    StatementResult clear_bindings() const;

    std::optional<int> column_int(int index) const;
    std::optional<int64_t> column_int64(int index) const;
    std::optional<double> column_double(int index) const;
    std::optional<bool> column_bool(int index) const;
    std::optional<std::string> column_text(int index) const;
    
    template <typename Type>
    std::optional<Type> column(int index) const;
};

} // namespace sequelite
```

<br>

<p align="right"><sub>[&nbsp;<a href="#sequelite-cpp">Back to Top</a> &nbsp;•&nbsp; <a href="#table-of-contents">Table of Contents</a>&nbsp;]</sub></p>

## 8. Error Handling
`sequelite-cpp` avoids exceptions, utilizing a predictable two-tier error paradigm:

1. **Creation Guards:** Factory functions that construct core state - such as `Database::create(std::string_view)` and `Statement::create(Database&, std::string_view)` - return an `std::optional<Database>` & `std::optional<Statement>` respectively, yielding `std::nullopt` on failure.

2. **Execution & Operation Status:** 
    - Operations like `db.execute()`, `stmt.bind()`, and `stmt.step()` return dedicated `Result<T>` structures.
    - Developers check success explicitly via lightweight status checks (e.g., if (!res.ok())), keeping control flow predictable and deterministic.

<br>

<p align="right"><sub>[&nbsp;<a href="#sequelite-cpp">Back to Top</a> &nbsp;•&nbsp; <a href="#table-of-contents">Table of Contents</a>&nbsp;]</sub></p>

<!-- ## 9. Ownership & Lifetime -->

<!-- ## 10. Thread Safety -->

<!-- ## 11. Comparison with Alternatives -->

<!-- ## 12. Limitations / Non-Goals -->

<!-- ## 13. Building & Running Tests -->

<!-- ## 14. Contributing / License -->
