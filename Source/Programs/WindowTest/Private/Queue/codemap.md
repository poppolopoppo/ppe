# Source/Programs/WindowTest/Private/Queue/

## Responsibility
This directory implements the Queue test subsystem, validating copy command operations between buffers and images, including bandwidth measurement, data pattern verification, and alignment boundary testing. These tests exercise the RHI's command queue submission and synchronization mechanisms.

## Design
The queue test definitions follow a consistent pattern: each test creates a source buffer and/or image, a destination buffer and/or image, and a copy command submission describing the copy operation. The tests cover simple buffer-to-buffer copies, image-to-image copies, and buffer-to-image or image-to-buffer cross-resource copies. Alignment boundaries are tested by specifying source and destination strides that cross DXT or texel alignment thresholds. Synchronization is provided via command list barriers and fence objects, and the tests verify that the copy completes correctly before the result is read back. The test results are checked via structured assertions that compare the copied data against expected patterns.

## Flow
The test flow begins when the test function creates the required RHI resources (buffers and/or images) with the appropriate flags and formats. The copy command is recorded into a command list, specifying the source, destination, and copy dimensions. The command list is submitted to the queue with appropriate synchronization (barriers or fences). The queue begins executing the command, and the test waits for the fence to signal. Once the fence signals, the test reads back the copied data from the destination resource and compares it against the expected pattern using structured assertions. The result is logged via `EXTERN_LOG_CATEGORY`, and the test function returns a pass/fail status.

## Integration
The Queue subsystem integrates with `Source/Runtime/RHI/` (`Source/Runtime/RHI`) for command queue submission and fence synchronization. It interfaces with `Source/Programs/WindowTest/Private/Drawing/` (`Source/Programs/WindowTest/Private/Drawing/codemap.md`) for tests that combine copy operations with graphics rendering. The test definitions are auto-discovered by the WindowTest harness at startup, and the results are reported through the harness's console summary.