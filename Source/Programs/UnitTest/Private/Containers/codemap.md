# Source/Programs/UnitTest/Private/Containers/

## Responsibility
This directory implements UnitTest test suites that exercise container smart pointer functionality and memory management patterns. These tests validate the `TPtrRef`, `TPtr`, and other smart pointer types used throughout the codebase for resource management.

## Design
The container test definitions create instances of `TPtrRef`, `TPtr`, and related smart pointer types, assigning them owned objects, shared objects, and null values. The tests verify that reference counting is correct, that null checks work as expected, and that assignment and copy semantics follow the documented patterns. Memory leak detection is verified by ensuring that all owned pointers are released at the end of the test scope. The tests cover both single-pointer scenarios and complex nested pointer graphs that appear in real-world usage patterns across the rendering and content pipeline subsystems.

## Flow
The test flow begins when the test function creates owned and shared smart pointer instances using the `TPtrRef` and `TPtr` constructors. The pointers are assigned to objects with varying lifetimes, and the test verifies that the reference counting and null-checking operations behave correctly. The test may also create nested pointer graphs, where one smart pointer owns another, and verify that destruction cascades correctly through the graph. After the pointer graph is established, the test function explicitly releases all owned pointers and checks that no memory leaks are detected. The result is logged via `EXTERN_LOG_CATEGORY`, and the test function returns a pass/fail status.

## Integration
The container tests integrate with `Source/Runtime/Serialize/` (`Source/Runtime/Serialize`) for smart pointer serialization and deserialization patterns, and with `Source/Runtime/RHI/Public/RHI` (`Source/Runtime/RTTI`) for RTTI-enabled pointer type validation. The test definitions are auto-discovered by the UnitTest runner at startup, and the results are reported through the runner's console summary.