# Source/Programs/UnitTest/Private/

## Responsibility
This directory contains the private implementation of the UnitTest test runner, including the console entry point, test discovery, test execution loop, and result reporting. These components are the operational core of the test runner.

## Design
The private implementation centers on the console application's `Main()` function, which initializes the test framework, discovers test programs via the `RTTI_TEST_REGISTRY`, and enters the console event loop. The `RunProgram()` method takes a test program identifier and dispatches to the appropriate test program implementation. Each test program is responsible for its own subsystem initialization (RHI, content pipeline, etc.), the test operation, and result reporting. The private implementation also handles progress reporting through `EXTERN_LOG_CATEGORY` structured logging, and cancellation via console event callbacks. The test registry is provided as a global variable, and each test program registers itself at static initialization time.

## Flow
The orchestration flow begins when the console entry point initializes the test framework and enters the console event loop. The user can select a test program from the menu, and the `RunProgram()` method dispatches to the appropriate test program. The selected test is loaded, its subsystems are initialized, the test operation is run, and the result is read back. The outcome is logged, and the console returns to the menu for further test selection. After all selected tests complete, the runner reports a summary of passed/failed tests and exits with an appropriate return code.

## Integration
The private implementation integrates with `Source/Programs/UnitTest/Public/` (`Source/Programs/UnitTest/Public/codemap.md`) for the test program interface and registry, and with `Source/Runtime/RHI/` (`Source/Runtime/RHI`) for tests that require graphics resources. It also interfaces with `Source/ContentPipeline/` (`Source/ContentPipeline`) for content pipeline integration tests. The test results are reported through the console and may be consumed by the CI pipeline for regression validation.