CC ?= cc
COVERAGE_CC ?= gcc
ANALYZER_CC ?= gcc
GCOV ?= gcov
CLANG_FORMAT ?= clang-format
CPPFLAGS ?=
CFLAGS ?= -O2
LDFLAGS ?=
LDLIBS ?=

STANDARD_FLAGS := -std=c17
WARNING_FLAGS := -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Werror
DEPENDENCY_FLAGS := -MMD -MP
SANITIZER_FLAGS := -g -O1 -fno-omit-frame-pointer -fsanitize=address,undefined
COVERAGE_FLAGS := -g -O0 --coverage
ANALYZER_FLAGS := -g -O0 -fanalyzer

# Isolated object trees prevent incompatible compiler flags from sharing artifacts.
BUILD_DIR := build
RELEASE_OBJECT_DIR := $(BUILD_DIR)/release/objects
SANITIZER_DIR := $(BUILD_DIR)/sanitize
SANITIZER_OBJECT_DIR := $(SANITIZER_DIR)/objects
SANITIZER_TEST_DIR := $(SANITIZER_DIR)/tests
COVERAGE_DIR := $(BUILD_DIR)/coverage
COVERAGE_OBJECT_DIR := $(COVERAGE_DIR)/objects
COVERAGE_TEST_DIR := $(COVERAGE_DIR)/tests
ANALYZER_OBJECT_DIR := $(BUILD_DIR)/analyzer/objects
TEST_DIR := $(BUILD_DIR)/tests

TARGET := binary-relation-analyzer
SANITIZER_TARGET := $(SANITIZER_DIR)/binary-relation-analyzer
COVERAGE_TARGET := $(COVERAGE_DIR)/binary-relation-analyzer

RELATION_TEST_TARGET := $(TEST_DIR)/relation-tests
ANALYSIS_TEST_TARGET := $(TEST_DIR)/analysis-tests
EXHAUSTIVE_TEST_TARGET := $(TEST_DIR)/exhaustive-tests
IO_REPORT_TEST_TARGET := $(TEST_DIR)/io-report-tests
TEST_TARGETS := $(RELATION_TEST_TARGET) $(ANALYSIS_TEST_TARGET) \
	$(EXHAUSTIVE_TEST_TARGET) $(IO_REPORT_TEST_TARGET)

SANITIZER_RELATION_TEST_TARGET := $(SANITIZER_TEST_DIR)/relation-tests
SANITIZER_ANALYSIS_TEST_TARGET := $(SANITIZER_TEST_DIR)/analysis-tests
SANITIZER_EXHAUSTIVE_TEST_TARGET := $(SANITIZER_TEST_DIR)/exhaustive-tests
SANITIZER_IO_REPORT_TEST_TARGET := $(SANITIZER_TEST_DIR)/io-report-tests
SANITIZER_TEST_TARGETS := $(SANITIZER_RELATION_TEST_TARGET) \
	$(SANITIZER_ANALYSIS_TEST_TARGET) $(SANITIZER_EXHAUSTIVE_TEST_TARGET) \
	$(SANITIZER_IO_REPORT_TEST_TARGET)

COVERAGE_RELATION_TEST_TARGET := $(COVERAGE_TEST_DIR)/relation-tests
COVERAGE_ANALYSIS_TEST_TARGET := $(COVERAGE_TEST_DIR)/analysis-tests
COVERAGE_EXHAUSTIVE_TEST_TARGET := $(COVERAGE_TEST_DIR)/exhaustive-tests
COVERAGE_IO_REPORT_TEST_TARGET := $(COVERAGE_TEST_DIR)/io-report-tests
COVERAGE_TEST_TARGETS := $(COVERAGE_RELATION_TEST_TARGET) $(COVERAGE_ANALYSIS_TEST_TARGET) \
	$(COVERAGE_EXHAUSTIVE_TEST_TARGET) $(COVERAGE_IO_REPORT_TEST_TARGET)

APP_SOURCE_NAMES := main application relation relation_analysis relation_closure relation_io \
	relation_report
TEST_SOURCE_NAMES := test_relation test_analysis test_exhaustive test_io_report
APP_SOURCES := $(addprefix src/,$(addsuffix .c,$(APP_SOURCE_NAMES)))
TEST_SOURCES := $(addprefix tests/,$(addsuffix .c,$(TEST_SOURCE_NAMES)))
HEADERS := $(wildcard include/*.h)
FORMAT_SOURCES := $(sort $(APP_SOURCES) $(TEST_SOURCES) $(HEADERS))

APP_OBJECTS := $(addprefix $(RELEASE_OBJECT_DIR)/src/,$(addsuffix .o,$(APP_SOURCE_NAMES)))
RELATION_OBJECT := $(RELEASE_OBJECT_DIR)/src/relation.o
ANALYSIS_OBJECTS := $(addprefix $(RELEASE_OBJECT_DIR)/src/,$(addsuffix .o,relation \
	relation_analysis relation_closure))
IO_REPORT_OBJECTS := $(addprefix $(RELEASE_OBJECT_DIR)/src/,$(addsuffix .o,relation \
	relation_analysis relation_closure relation_io relation_report))

SANITIZER_APP_OBJECTS := $(addprefix $(SANITIZER_OBJECT_DIR)/src/, \
	$(addsuffix .o,$(APP_SOURCE_NAMES)))
SANITIZER_RELATION_OBJECT := $(SANITIZER_OBJECT_DIR)/src/relation.o
SANITIZER_ANALYSIS_OBJECTS := $(addprefix $(SANITIZER_OBJECT_DIR)/src/, \
	$(addsuffix .o,relation relation_analysis relation_closure))
SANITIZER_IO_REPORT_OBJECTS := $(addprefix $(SANITIZER_OBJECT_DIR)/src/, \
	$(addsuffix .o,relation relation_analysis relation_closure relation_io relation_report))

COVERAGE_APP_OBJECTS := $(addprefix $(COVERAGE_OBJECT_DIR)/src/, \
	$(addsuffix .o,$(APP_SOURCE_NAMES)))
COVERAGE_RELATION_OBJECT := $(COVERAGE_OBJECT_DIR)/src/relation.o
COVERAGE_ANALYSIS_OBJECTS := $(addprefix $(COVERAGE_OBJECT_DIR)/src/, \
	$(addsuffix .o,relation relation_analysis relation_closure))
COVERAGE_IO_REPORT_OBJECTS := $(addprefix $(COVERAGE_OBJECT_DIR)/src/, \
	$(addsuffix .o,relation relation_analysis relation_closure relation_io relation_report))

ANALYZER_OBJECTS := $(addprefix $(ANALYZER_OBJECT_DIR)/src/, \
	$(addsuffix .o,$(APP_SOURCE_NAMES)))

RELEASE_TEST_OBJECTS := $(addprefix $(RELEASE_OBJECT_DIR)/tests/, \
	$(addsuffix .o,$(TEST_SOURCE_NAMES)))
SANITIZER_TEST_OBJECTS := $(addprefix $(SANITIZER_OBJECT_DIR)/tests/, \
	$(addsuffix .o,$(TEST_SOURCE_NAMES)))
COVERAGE_TEST_OBJECTS := $(addprefix $(COVERAGE_OBJECT_DIR)/tests/, \
	$(addsuffix .o,$(TEST_SOURCE_NAMES)))
DEPENDENCY_FILES := $(APP_OBJECTS:.o=.d) $(RELEASE_TEST_OBJECTS:.o=.d) \
	$(SANITIZER_APP_OBJECTS:.o=.d) $(SANITIZER_TEST_OBJECTS:.o=.d) \
	$(COVERAGE_APP_OBJECTS:.o=.d) $(COVERAGE_TEST_OBJECTS:.o=.d) \
	$(ANALYZER_OBJECTS:.o=.d)

.PHONY: all test sanitize coverage analyze format check-format clean

all: $(TARGET)

$(RELEASE_OBJECT_DIR)/src/%.o: src/%.c
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) -Iinclude $(STANDARD_FLAGS) $(WARNING_FLAGS) $(CFLAGS) \
		$(DEPENDENCY_FLAGS) -c $< -o $@

$(RELEASE_OBJECT_DIR)/tests/%.o: tests/%.c
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) -Iinclude $(STANDARD_FLAGS) $(WARNING_FLAGS) $(CFLAGS) \
		$(DEPENDENCY_FLAGS) -c $< -o $@

$(SANITIZER_OBJECT_DIR)/src/%.o: src/%.c
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) -Iinclude $(STANDARD_FLAGS) $(WARNING_FLAGS) $(SANITIZER_FLAGS) \
		$(DEPENDENCY_FLAGS) -c $< -o $@

$(SANITIZER_OBJECT_DIR)/tests/%.o: tests/%.c
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) -Iinclude $(STANDARD_FLAGS) $(WARNING_FLAGS) $(SANITIZER_FLAGS) \
		$(DEPENDENCY_FLAGS) -c $< -o $@

$(COVERAGE_OBJECT_DIR)/src/%.o: src/%.c
	@mkdir -p $(@D)
	$(COVERAGE_CC) $(CPPFLAGS) -Iinclude $(STANDARD_FLAGS) $(WARNING_FLAGS) $(COVERAGE_FLAGS) \
		$(DEPENDENCY_FLAGS) -c $< -o $@

$(COVERAGE_OBJECT_DIR)/tests/%.o: tests/%.c
	@mkdir -p $(@D)
	$(COVERAGE_CC) $(CPPFLAGS) -Iinclude $(STANDARD_FLAGS) $(WARNING_FLAGS) $(COVERAGE_FLAGS) \
		$(DEPENDENCY_FLAGS) -c $< -o $@

$(ANALYZER_OBJECT_DIR)/src/%.o: src/%.c
	@mkdir -p $(@D)
	$(ANALYZER_CC) $(CPPFLAGS) -Iinclude $(STANDARD_FLAGS) $(WARNING_FLAGS) $(ANALYZER_FLAGS) \
		$(DEPENDENCY_FLAGS) -c $< -o $@

$(TARGET): $(APP_OBJECTS)
	$(CC) $(APP_OBJECTS) $(LDFLAGS) $(LDLIBS) -o $@

$(SANITIZER_TARGET): $(SANITIZER_APP_OBJECTS)
	@mkdir -p $(@D)
	$(CC) $(SANITIZER_FLAGS) $(SANITIZER_APP_OBJECTS) $(LDFLAGS) $(LDLIBS) -o $@

$(COVERAGE_TARGET): $(COVERAGE_APP_OBJECTS)
	@mkdir -p $(@D)
	$(COVERAGE_CC) $(COVERAGE_FLAGS) $(COVERAGE_APP_OBJECTS) $(LDFLAGS) $(LDLIBS) -o $@

$(RELATION_TEST_TARGET): $(RELEASE_OBJECT_DIR)/tests/test_relation.o $(RELATION_OBJECT)
	@mkdir -p $(@D)
	$(CC) $^ $(LDFLAGS) $(LDLIBS) -o $@

$(ANALYSIS_TEST_TARGET): $(RELEASE_OBJECT_DIR)/tests/test_analysis.o $(ANALYSIS_OBJECTS)
	@mkdir -p $(@D)
	$(CC) $^ $(LDFLAGS) $(LDLIBS) -o $@

$(EXHAUSTIVE_TEST_TARGET): $(RELEASE_OBJECT_DIR)/tests/test_exhaustive.o $(ANALYSIS_OBJECTS)
	@mkdir -p $(@D)
	$(CC) $^ $(LDFLAGS) $(LDLIBS) -o $@

$(IO_REPORT_TEST_TARGET): $(RELEASE_OBJECT_DIR)/tests/test_io_report.o $(IO_REPORT_OBJECTS)
	@mkdir -p $(@D)
	$(CC) $^ $(LDFLAGS) $(LDLIBS) -o $@

$(SANITIZER_RELATION_TEST_TARGET): $(SANITIZER_OBJECT_DIR)/tests/test_relation.o \
	$(SANITIZER_RELATION_OBJECT)
	@mkdir -p $(@D)
	$(CC) $(SANITIZER_FLAGS) $^ $(LDFLAGS) $(LDLIBS) -o $@

$(SANITIZER_ANALYSIS_TEST_TARGET): $(SANITIZER_OBJECT_DIR)/tests/test_analysis.o \
	$(SANITIZER_ANALYSIS_OBJECTS)
	@mkdir -p $(@D)
	$(CC) $(SANITIZER_FLAGS) $^ $(LDFLAGS) $(LDLIBS) -o $@

$(SANITIZER_EXHAUSTIVE_TEST_TARGET): $(SANITIZER_OBJECT_DIR)/tests/test_exhaustive.o \
	$(SANITIZER_ANALYSIS_OBJECTS)
	@mkdir -p $(@D)
	$(CC) $(SANITIZER_FLAGS) $^ $(LDFLAGS) $(LDLIBS) -o $@

$(SANITIZER_IO_REPORT_TEST_TARGET): $(SANITIZER_OBJECT_DIR)/tests/test_io_report.o \
	$(SANITIZER_IO_REPORT_OBJECTS)
	@mkdir -p $(@D)
	$(CC) $(SANITIZER_FLAGS) $^ $(LDFLAGS) $(LDLIBS) -o $@

$(COVERAGE_RELATION_TEST_TARGET): $(COVERAGE_OBJECT_DIR)/tests/test_relation.o \
	$(COVERAGE_RELATION_OBJECT)
	@mkdir -p $(@D)
	$(COVERAGE_CC) $(COVERAGE_FLAGS) $^ $(LDFLAGS) $(LDLIBS) -o $@

$(COVERAGE_ANALYSIS_TEST_TARGET): $(COVERAGE_OBJECT_DIR)/tests/test_analysis.o \
	$(COVERAGE_ANALYSIS_OBJECTS)
	@mkdir -p $(@D)
	$(COVERAGE_CC) $(COVERAGE_FLAGS) $^ $(LDFLAGS) $(LDLIBS) -o $@

$(COVERAGE_EXHAUSTIVE_TEST_TARGET): $(COVERAGE_OBJECT_DIR)/tests/test_exhaustive.o \
	$(COVERAGE_ANALYSIS_OBJECTS)
	@mkdir -p $(@D)
	$(COVERAGE_CC) $(COVERAGE_FLAGS) $^ $(LDFLAGS) $(LDLIBS) -o $@

$(COVERAGE_IO_REPORT_TEST_TARGET): $(COVERAGE_OBJECT_DIR)/tests/test_io_report.o \
	$(COVERAGE_IO_REPORT_OBJECTS)
	@mkdir -p $(@D)
	$(COVERAGE_CC) $(COVERAGE_FLAGS) $^ $(LDFLAGS) $(LDLIBS) -o $@

test: $(TARGET) $(TEST_TARGETS)
	./$(RELATION_TEST_TARGET)
	./$(ANALYSIS_TEST_TARGET)
	./$(EXHAUSTIVE_TEST_TARGET)
	./$(IO_REPORT_TEST_TARGET)
	./tests/run_tests.sh ./$(TARGET)

sanitize: $(SANITIZER_TARGET) $(SANITIZER_TEST_TARGETS)
	ASAN_OPTIONS=detect_leaks=$${ASAN_DETECT_LEAKS:-0} ./$(SANITIZER_RELATION_TEST_TARGET)
	ASAN_OPTIONS=detect_leaks=$${ASAN_DETECT_LEAKS:-0} ./$(SANITIZER_ANALYSIS_TEST_TARGET)
	ASAN_OPTIONS=detect_leaks=$${ASAN_DETECT_LEAKS:-0} ./$(SANITIZER_EXHAUSTIVE_TEST_TARGET)
	ASAN_OPTIONS=detect_leaks=$${ASAN_DETECT_LEAKS:-0} ./$(SANITIZER_IO_REPORT_TEST_TARGET)
	ASAN_OPTIONS=detect_leaks=$${ASAN_DETECT_LEAKS:-0} ./tests/run_tests.sh ./$(SANITIZER_TARGET)

coverage: $(COVERAGE_TARGET) $(COVERAGE_TEST_TARGETS)
	./$(COVERAGE_RELATION_TEST_TARGET)
	./$(COVERAGE_ANALYSIS_TEST_TARGET)
	./$(COVERAGE_EXHAUSTIVE_TEST_TARGET)
	./$(COVERAGE_IO_REPORT_TEST_TARGET)
	./tests/run_tests.sh ./$(COVERAGE_TARGET)
	$(GCOV) --branch-counts --branch-probabilities --function-summaries --no-output \
		--object-directory $(COVERAGE_OBJECT_DIR)/src $(APP_SOURCES)

analyze: $(ANALYZER_OBJECTS)

format:
	@command -v $(CLANG_FORMAT) >/dev/null 2>&1 || \
		{ echo "Error: $(CLANG_FORMAT) is required for formatting" >&2; exit 1; }
	$(CLANG_FORMAT) -i $(FORMAT_SOURCES)

check-format:
	@command -v $(CLANG_FORMAT) >/dev/null 2>&1 || \
		{ echo "Error: $(CLANG_FORMAT) is required for format checks" >&2; exit 1; }
	$(CLANG_FORMAT) --dry-run --Werror $(FORMAT_SOURCES)

clean:
	rm -rf $(BUILD_DIR)
	rm -f $(TARGET)

# Compiler-generated dependency files keep incremental builds header-aware.
-include $(DEPENDENCY_FILES)
