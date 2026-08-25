# Source/Programs/WindowTest/Public/

## Responsibility
The Public directory for WindowTest exposes the test framework types and RHI test interface definitions. These types are intended for use by developers adding new RHI tests or integrating the test harness into CI pipelines.

## Design
The public API centers on the `IWindowTestSession` interface, which defines the contract for a test session: initialize, run individual tests, and report results. The `FTestResult` struct encapsulates pass/fail status, error messages, and timing information for each test operation. RHI test descriptor types (`FDrawTestDesc`, `FComputeTestDesc`, `FRayTracingTestDesc`) define the parameters for each test category, including resource requirements, iteration counts, and validation thresholds. All public types are designed to be RHI-agnostic, depending only on the base `RHI::IFrameGraph` interface and the application window foundation. The test registry macro (`RTTI_TEST_REGISTRY`) enables automatic test discovery at harness startup.

## Flow
External consumers implement the `IWindowTestSession` interface and register test descriptors using the `RTTI_TEST_REGISTRY` macro. The WindowTest harness reads the registry at startup, discovers all registered tests, and presents a menu for the user to select which tests to run. Each test is executed by calling the session's `RunTest()` method, which initializes the required RHI resources, executes the test operation, and returns an `FTestResult`. Results are collected and displayed in the console summary at the end of the session. The test registry is populated via static initialization in individual test file headers, making it easy to add new tests without modifying the harness core.

## Integration
The public types are consumed by CI scripts and external test frameworks that need to drive RHI regression validation. The test registry integrates with `Source/Programs/WindowTest/Private/` (`Source/Programs/WindowTest/Private/codemap.md`) for automatic test discovery at harness startup. The `IWindowTestSession` implementation integrates with `Source/Runtime/RHI/` (`Source/Runtime/RHI`) for resource creation and validation. Downstream, test results feed into the CI pipeline and may trigger rebuilds via `Source/Programs/BuildRobot/` (`Source/Programs/BuildRobot/codemap.md`).