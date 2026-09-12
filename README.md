# Binary Relation Analyzer

Binary Relation Analyzer is a dependency-free C17 command-line application for inspecting finite
binary relations. It reads a set and its ordered pairs from a text file, displays the corresponding
matrix, evaluates common relation properties, and computes reflexive, symmetric, and transitive
closures.

## Features

- Evaluates reflexivity, irreflexivity, symmetry, antisymmetry, asymmetry, and transitivity.
- Identifies equivalence relations and partial orders.
- Reports counterexamples for reflexivity, irreflexivity, symmetry, antisymmetry, and transitivity.
- Computes minimal reflexive, symmetric, and transitive supersets.
- Accepts signed integer labels and ignores repeated ordered pairs.
- Handles invalid input with line-specific diagnostics and a nonzero exit status.
- Includes focused unit tests, CLI integration tests, and exhaustive verification of every relation
  over sets containing up to three elements.

## Requirements

To build and run the application, you need:

- a C17 compiler such as GCC or Clang;
- GNU Make;
- a POSIX-compatible shell for the integration tests.

Optional development commands also use GCC, gcov, and clang-format.

## Build and run

Build the release executable:

```console
$ make
```

Analyze the bundled example:

```console
$ ./binary-relation-analyzer examples/sample.txt
```

The report contains the relation matrix, the result of each property check, relevant
counterexamples, and all supported closures.

Display the command-line help with:

```console
$ ./binary-relation-analyzer --help
```

Use `--` before a filename that begins with a dash:

```console
$ ./binary-relation-analyzer -- -relation.txt
```

## Input format

The first line declares the number of elements followed by exactly that many unique integer
labels. Each subsequent non-empty line contains one ordered pair:

```text
3 10 20 30
10 20
20 30
```

This input represents the set `{10, 20, 30}` and the relation `{(10, 20), (20, 30)}`.

Input rules:

- A set may contain from 0 to 50 elements.
- Labels must be unique signed integers representable by the platform's C `int` type.
- Whitespace separates values, and blank lines after the header are ignored.
- Every ordered-pair member must belong to the declared set.
- Each pair must occupy its own line and contain exactly two labels.
- Repeated pairs are accepted but do not change the relation.
- Missing, extra, malformed, unknown, or duplicate element values invalidate the entire input.

An empty relation over an empty set can be written as:

```text
0
```

## Properties

For a relation `R` over a finite set `A`, the analyzer uses the following definitions:

| Property | Condition |
| --- | --- |
| Reflexive | For every `a` in `A`, `(a, a)` belongs to `R`. |
| Irreflexive | For every `a` in `A`, `(a, a)` does not belong to `R`. |
| Symmetric | If `(a, b)` belongs to `R`, then `(b, a)` belongs to `R`. |
| Antisymmetric | If `(a, b)` and `(b, a)` belong to `R`, then `a = b`. |
| Asymmetric | If `(a, b)` belongs to `R`, then `(b, a)` does not belong to `R`. |
| Transitive | If `(a, b)` and `(b, c)` belong to `R`, then `(a, c)` belongs to `R`. |
| Equivalence relation | `R` is reflexive, symmetric, and transitive. |
| Partial order | `R` is reflexive, antisymmetric, and transitive. |

Properties of the empty set follow vacuous-truth semantics. Its empty relation therefore satisfies
all six base properties and is both an equivalence relation and a partial order.

Each closure is a new relation containing every original pair plus the minimum pairs required to
satisfy the corresponding property. The transitive closure is computed with Warshall's algorithm.

## Development commands

| Command | Purpose |
| --- | --- |
| `make test` | Build and run unit, exhaustive, integration, parser, and CLI tests. |
| `make sanitize` | Run the complete suite with AddressSanitizer and UBSan. |
| `make coverage` | Run instrumented tests and print gcov line, branch, and function coverage. |
| `make analyze` | Compile all application modules with the GCC static analyzer. |
| `make format` | Format all C sources and public headers with clang-format. |
| `make check-format` | Fail if any C source or public header is not correctly formatted. |
| `make clean` | Remove the executable and all generated build artifacts. |

LeakSanitizer is disabled by default because it is not supported in some traced or sandboxed
environments. Enable it on a compatible host with:

```console
$ ASAN_DETECT_LEAKS=1 make sanitize
```

Override the release compiler when needed:

```console
$ make clean
$ make CC=clang test
```

Generated objects, dependency files, test executables, and instrumented binaries are isolated under
`build/`. The release executable remains at the repository root.

## Project structure

```text
.
├── examples/                 Sample input files
├── include/                  Public module interfaces
├── src/
│   ├── main.c                Process entry point
│   ├── application.c         Command-line parsing and application lifecycle
│   ├── relation.c            Relation domain model and pair storage
│   ├── relation_analysis.c   Pure property evaluation
│   ├── relation_closure.c    Closure algorithms
│   ├── relation_io.c         Text input parsing and validation
│   └── relation_report.c     Deterministic report formatting
└── tests/                    Unit, exhaustive, fixture, and integration tests
```

The `Relation` type is opaque. Analysis functions do not modify it, and each closure function
returns an independently owned relation. This separation keeps parsing, domain logic, algorithms,
and presentation testable without invoking the full application.

## Continuous integration

GitHub Actions builds and tests the project with GCC and Clang on every push and pull request. A
separate sanitizer job checks memory and undefined behavior, while the quality job verifies source
formatting, runs GCC static analysis, and collects coverage.

## Author

Created by Lucas Paulo Martins Mariz.
