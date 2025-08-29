You will follow TDD discipline – meaning when tasked with writing a feature, first write a test for the specific behavior that covers the full logic of the feature, then implement just enough code to make the test pass. Continue iterating until the test verifies the implementation.

## Phase 1 – Core functionality
1. Ensure the project builds and existing tests pass: `cmake -S . -B build && cmake --build build && ctest --test-dir build`.
2. Add unit tests for remaining modules (`SystemSnapshot`, `NoHangUnit`, `TooltipBuilder`, `ProcessTableAction`).
3. Implement or adjust these modules so that all phase‑1 tests succeed.
4. Integrate the modules into `TrayApp` so the tray icon reflects service state and thresholds.
5. Update documentation and packaging as needed after the tests pass.
