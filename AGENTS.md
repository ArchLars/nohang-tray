# AGENTS.md

A collaborative workflow for using AI agents to assist development of **nohang-tray**, centered on strict test driven development. This document defines roles, handoffs, and a protocol that requires writing a failing test first, then writing the minimal code to pass, then refactoring. The cycle is commonly known as red, green, refactor. ([martinfowler.com][1])

---

## 1) Purpose and scope

* Ensure that every change is driven by an executable test, not by prose requirements.
* Make agent responsibilities explicit, with small, auditable steps.
* Keep the stack practical for this codebase, which uses CMake, CTest, C++ with Qt 6 and KDE Frameworks. Unit tests use GoogleTest for core logic and Qt Test for Qt specific behavior. ([Google GitHub][2], [Qt Documentation][3])

---

## 2) Guiding principles

1. **Write the test first**, then run it and confirm it fails.
2. **Make the test pass** by writing the smallest possible code.
3. **Refactor on green**, keep behavior unchanged, improve design.
   This sequencing is the core TDD loop. It also encourages test list grooming, where you add new tests as you discover them, then pick the next one to drive the next small step. ([martinfowler.com][1], [stanislaw.github.io][4], [tidyfirst.substack.com][5])

---

## 3) Agent roles

### Test Author Agent

* Chooses one behavior, writes a minimal test that will fail on current code, includes at least one boundary case.
* Registers the test with CMake and CTest, runs locally, pastes the failing output in the PR. ([CMake][6])

### Implementer Agent

* Reads the failing output, writes the smallest change that makes the test pass, runs CTest and posts the green summary. Avoids feature creep.

### Refactorer Agent

* Improves structure and names without changing behavior, runs tests again to confirm green.

### Reviewer Agent

* Verifies that a test existed and failed before implementation, checks that commits or PR notes reflect red, green, refactor, ensures CI is green, merges. A concise checklist improves reliability. ([blog.ploeh.dk][7])

### Docs Agent

* Updates README or API docs when visible behavior changes, links the test name or case ID as the single source of truth.

---

## 4) TDD protocol, step by step

1. **Pick one behavior**
   Keep scope small, for example one function or a tiny class method. Write a short test plan in the PR body.

2. **Write the test**

   * For core logic, prefer GoogleTest. For Qt types or GUI event handling, use Qt Test. Both integrate cleanly with CTest since tests are executables with exit codes. ([Google GitHub][2], [Qt Documentation][3])
   * Place tests under `tests/`, wire them in CMake using `enable_testing()` and `add_test`. ([CMake][8])

3. **Confirm red**
   Build and run tests. The new test must fail. Capture the exact failure message in the PR. This proves the test executes and targets the right behavior. The canonical loop is red, green, refactor. ([martinfowler.com][1])

4. **Make it green**
   Implement only what is required to pass. Rebuild and run `ctest`. Paste the passing summary. ([CMake][9])

5. **Refactor**
   Improve names and structure while tests stay green. Commit refactors separately. This preserves intent and enables fast review. ([IBM][10])

6. **Repeat**
   Pick the next test from your test list. Proper sequencing is a learned skill, start with the simplest case, then add edge cases. ([martinfowler.com][1])

---

## 5) Tooling stack and conventions

* **GoogleTest**

  * Add via your preferred method, then follow the official CMake Quickstart example for discovery and linking. Store tests under `tests/`, name files `*_test.cpp`. ([Google GitHub][2])

* **Qt Test**

  * Use `Qt6::Test`, write data driven tests when helpful, simulate signals and events where needed. ([Qt Documentation][3])

* **CTest integration**

  * In the project root CMakeLists, `enable_testing()` or `include(CTest)`, then register each test with `add_test(NAME ... COMMAND ...)`. CI runs `ctest` to discover and execute tests. ([CMake][8])

* **Targets and naming**

  * Test targets should be small, focused executables. Prefer one logical subject per test file. Use clear names like `systemsnapshot_parse_test`.

---

## 6) Agent handoffs and prompt templates

**Test Author Agent, prompt outline**

* Input: feature request, linked issue.
* Output: one failing test, CMake registration, local failing output.
* Checklist: smallest behavior, edge case included, test compiles, test fails.

**Implementer Agent, prompt outline**

* Input: failing test and output.
* Output: minimal code change, passing CTest run, brief note of what changed.

**Refactorer Agent, prompt outline**

* Input: green test run.
* Output: refactoring commits, unchanged behavior, green test run repeated.

**Reviewer Agent, checklist**

* A failing test preceded code.
* The green change is minimal.
* Refactors are separate commits.
* CI is green and tests are registered with CTest. ([CMake][6])

---

## 7) CI policy

* CI builds with CMake and runs `ctest`. No merge if tests fail.
* New behavior requires a new test.
* All tests must be registered with CTest, or CI will not run them. ([CMake][9])

---

## 8) Directory layout

```
src/            production code
tests/          unit tests (GoogleTest and Qt Test)
CMakeLists.txt  build configuration, includes testing
cmake/          optional CMake helpers
```

---

## 9) Local commands

* Configure and build: `cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug && cmake --build build -j`
* Run tests: `ctest --test-dir build --output-on-failure`
* Filter tests: `ctest --test-dir build -R <name-part>` ([CMake][9])

---

## 10) Definition of done

* A failing test existed first.
* The smallest code change made it pass.
* Refactors kept tests green.
* CI is green.
* Docs updated when public behavior changed.

---

## 11) Notes on test design

* Start with a simple happy path, then add edge cases.
* For Qt code, consider data driven tests for parameter sweeps, but keep readability high. ([Qt Documentation][11])
* Keep tests deterministic and fast. Isolate file system and process calls behind small adapters.

---

## 12) Rationale and further reading

* **Red, green, refactor** overview and sequencing guidance. ([martinfowler.com][1])
* **Kent Beck on TDD**, the rule of writing the failing test first, then making it pass, then refactoring. ([stanislaw.github.io][4])
* **GoogleTest with CMake**, official quickstart. ([Google GitHub][2])
* **CTest and add\_test**, how CMake discovers and runs tests. ([CMake][9])
* **Qt Test** overview and tutorial for Qt specific testing. ([Qt Documentation][3])

This workflow treats tests as the first class specification, then uses them to drive implementation and refactoring.

[1]: https://martinfowler.com/bliki/TestDrivenDevelopment.html?utm_source=chatgpt.com "Test Driven Development"
[2]: https://google.github.io/googletest/quickstart-cmake.html?utm_source=chatgpt.com "Quickstart: Building with CMake | GoogleTest"
[3]: https://doc.qt.io/qt-6/qtest-overview.html?utm_source=chatgpt.com "Qt Test Overview"
[4]: https://stanislaw.github.io/2016-01-25-notes-on-test-driven-development-by-example-by-kent-beck.html?utm_source=chatgpt.com "Notes on \"Test-Driven Development by Example\" by Kent Beck"
[5]: https://tidyfirst.substack.com/p/canon-tdd?utm_source=chatgpt.com "Canon TDD - by Kent Beck - Software Design: Tidy First?"
[6]: https://cmake.org/cmake/help/latest/command/add_test.html?utm_source=chatgpt.com "add_test — CMake 4.1.0 Documentation"
[7]: https://blog.ploeh.dk/2019/10/21/a-red-green-refactor-checklist/?utm_source=chatgpt.com "A red-green-refactor checklist - ploeh blog"
[8]: https://cmake.org/cmake/help/book/mastering-cmake/chapter/Testing%20With%20CMake%20and%20CTest.html?utm_source=chatgpt.com "Testing With CMake and CTest"
[9]: https://cmake.org/cmake/help/latest/manual/ctest.1.html?utm_source=chatgpt.com "ctest(1) — CMake 4.1.0 Documentation"
[10]: https://www.ibm.com/think/topics/code-refactoring?utm_source=chatgpt.com "What Is Code Refactoring?"
[11]: https://doc.qt.io/qt-6/qttestlib-tutorial2-example.html?utm_source=chatgpt.com "Chapter 2: Data Driven Testing"
