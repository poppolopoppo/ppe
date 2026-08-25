# Source/Programs/UnitTest/Public/

## Responsibility
The Public directory for UnitTest exposes the test runner interface and test program base class definitions. These types are intended for use by developers adding new test programs or integrating the test runner into CI pipelines.

## Design
The public API centers on the `IUnitTestProgram` interface, which defines the contract for a test program: initialize, run, and report results. The `FTestResult` struct encapsulates pass/fail status, error messages, and timing information. The test runner console entry point and configuration types are also exposed, enabling external scripts to drive the test runner headlessly. All public types are designed to be subsystem-agnostic, depending only on the base application framework and the `EXTERN_LOG_CATEGORY` macro for structured logging. The `RTTI_TEST_REGISTRY` macro is exposed for automatic test discovery.

## Flow
External consumers implement the `IUnitTestProgram` interface and register their program using the `RTTI_TEST_REGISTRY` macro. The UnitTest console reads the registry at startup, discovers all registered programs, and presents a menu for the user to select which tests to run. Each program is executed by the runner's main loop, which calls `Initialize()`, `Run()`, and reports the `FTestResult`. Results are collected and displayed in the console summary at the end of the session. The test registry is populated via static initialization in individual test program headers, making it easy to add new programs without modifying the runner core.

## Integration
The public types are consumed by CI scripts and external test frameworks that need to drive the test runner headlessly. The test registry integrates with `Source/Programs/UnitTest/Private/` (`Source/Programs/UnitTest/Private/codemap.md`) for automatic program discovery at runner startup. The `IUnitTestProgram` implementation integrates with `Source/Runtime/RHI/` (`Source/Runtime/RHI`) for tests that require graphics resources, and with `Source/ContentPipeline/` (`Source/ContentPipeline`) for content pipeline integration tests. Downstream, test results feed into the CI pipeline and may trigger rebuilds via `Source/Programs/BuildRobot/` (`Source/Programs/BuildRobot/codemap.md`).