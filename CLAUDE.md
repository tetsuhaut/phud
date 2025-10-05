# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Communication
- Respond to the user in French

## Project Overview

PHUD (Poker Head-Up Display) is a C++23 desktop application that provides real-time poker statistics overlay for supported poker sites. The project uses CMake for build management and supports both Visual Studio 2022 and GCC/Ninja build systems on Windows.

## Build System

### Prerequisites
The following environment variables must be set before building:
- `Boost_DIR` - Path to Boost library installation
- `FLTK_DIR` - Path to FLTK GUI library installation  
- `frozen_DIR` - Path to frozen library installation
- `Microsoft.GSL_DIR` - Path to Microsoft GSL library installation
- `SPDLOG_DIR` - Path to spdlog library installation
- `SQLite3_DIR` - Path to SQLite3 library installation
- `stlab_DIR` - Path to stlab library installation
- `SCRIPTS_DIR` - Path to build scripts directory

### Build Commands

**For GCC/Ninja (recommended):**
```batch
build-gcc.bat
```

**For Visual Studio 2022:**
```batch
build-vs2022.bat
```

**Generated helper scripts (in build directory):**
- `make.bat` - Build with ninja
- `makeTest.bat` - Build and run all tests
- `runAllTests.bat` - Run all unit tests
- `runOneTest.bat [test_name]` - Run specific test
- `phud.bat` - Launch main application
- `guiDryRun.bat` - Launch GUI dry run
- `updateDb.bat` - Update test databases

### Code Quality Tools

**Format code (AStyle):**
```batch
format.bat
```

**Static analysis (cppcheck):**
```batch
static_check.bat
```

## Architecture

### Core Components

- **mainLib** - Main application library containing shared functionality
- **phud** - Primary GUI executable (Windows application, no console)
- **dbgen** - Database generator utility for test data
- **guiDryRun** - GUI testing utility
- **unitTests** - Boost.Test-based unit test suite

### Key Modules

- **entities/** - Core domain objects (Player, Card, Hand, Game, etc.)
- **gui/** - FLTK-based GUI components and overlays
- **db/** - SQLite database layer and SQL operations
- **history/** - Poker site hand history parsers (Winamax, PMU)
- **statistics/** - Player and table statistics calculation
- **threads/** - Thread management and async operations
- **filesystem/** - File watching and I/O utilities

### Dependencies

- **C++23** standard with modern features
- **FLTK 1.4.4+** for GUI
- **Boost 1.88.0+** (unit_test_framework)
- **SQLite3** for data persistence
- **spdlog 1.15.3+** for logging
- **Microsoft GSL** for guidelines support library
- **stlab 2.1.1** for concurrency utilities
- **frozen 1.1.0** for compile-time containers

## Development Practices

### Code Style
- Uses AStyle with Java-style formatting
- 2-space indentation, 100-character line limit
- Extensive compiler warnings enabled (-Wall, -Wextra, -Wpedantic for GCC)
- Static linking preferred for deployment
- Prefer `auto` for type deduction when the type is obvious from context
- Use `const` everywhere possible to ensure immutability
- Use `constexpr` for compile-time constants and functions
- Prefer brace initialization `{}` over assignment `=` for all variable declarations
  - Example: `const auto value { 42 };` instead of `const auto value = 42;`

### Testing
- Unit tests use Boost.Test framework
- Test resources in `src/test/resources/`
- Run tests with generated batch files in build directory

### Logging
- Uses spdlog for structured logging
- Logger instances created per source file
- UTF-8 encoding throughout codebase