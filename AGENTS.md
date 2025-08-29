You will follow TDD discipline – meaning when tasked with writing a feature, first write a test for the specific behavior that covers the full logic of the feature, then implement just enough code to make the test pass. Continue iterating until the test verifies the implementation. Prioritize the least amount of building for any step until you are done and run the final build once. Ninja and development efficiency should be used regardless. Build with debug info and flags on during construction of the code and testing of it, then without at the end.

Refer to `CODING_GUIDE.md` for an overview of the project structure, build, and test commands.

To compile a specific target, run `cmake --build build --target <target_name>` (e.g., `cmake --build build --target Thresholds_test`).

Update `README` and other relevant documentation after any changes.

Install `ccache` and configure its maximum size (e.g., run `ccache -M 5G`) to speed up rebuilds.

