CC ?= cc
CPPFLAGS ?=
CFLAGS ?= -O2
LDFLAGS ?=
LDLIBS ?=

STANDARD_FLAGS := -std=c17
WARNING_FLAGS := -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Werror
SANITIZER_FLAGS := -g -O1 -fno-omit-frame-pointer -fsanitize=address,undefined

TARGET := a.out
SANITIZER_TARGET := a.out-sanitize
RELATION_TEST_TARGET := relation-tests
SANITIZER_RELATION_TEST_TARGET := relation-tests-sanitize
ANALYSIS_TEST_TARGET := analysis-tests
SANITIZER_ANALYSIS_TEST_TARGET := analysis-tests-sanitize
APP_SOURCES := src/main.c src/application.c src/relation.c src/relation_analysis.c \
	src/relation_closure.c src/relation_io.c src/relation_report.c
RELATION_SOURCE := src/relation.c
ANALYSIS_SOURCES := src/relation.c src/relation_analysis.c src/relation_closure.c
HEADERS := $(wildcard include/*.h)

.PHONY: all test sanitize clean

all: $(TARGET)

$(TARGET): $(APP_SOURCES) $(HEADERS)
	$(CC) $(CPPFLAGS) -Iinclude $(STANDARD_FLAGS) $(WARNING_FLAGS) $(CFLAGS) $(APP_SOURCES) $(LDFLAGS) $(LDLIBS) -o $@

$(SANITIZER_TARGET): $(APP_SOURCES) $(HEADERS)
	$(CC) $(CPPFLAGS) -Iinclude $(STANDARD_FLAGS) $(WARNING_FLAGS) $(SANITIZER_FLAGS) $(APP_SOURCES) $(LDFLAGS) $(LDLIBS) -o $@


$(RELATION_TEST_TARGET): tests/test_relation.c $(RELATION_SOURCE) include/relation.h
	$(CC) $(CPPFLAGS) -Iinclude $(STANDARD_FLAGS) $(WARNING_FLAGS) $(CFLAGS) tests/test_relation.c $(RELATION_SOURCE) $(LDFLAGS) $(LDLIBS) -o $@

$(SANITIZER_RELATION_TEST_TARGET): tests/test_relation.c $(RELATION_SOURCE) include/relation.h
	$(CC) $(CPPFLAGS) -Iinclude $(STANDARD_FLAGS) $(WARNING_FLAGS) $(SANITIZER_FLAGS) tests/test_relation.c $(RELATION_SOURCE) $(LDFLAGS) $(LDLIBS) -o $@

$(ANALYSIS_TEST_TARGET): tests/test_analysis.c $(ANALYSIS_SOURCES) $(HEADERS)
	$(CC) $(CPPFLAGS) -Iinclude $(STANDARD_FLAGS) $(WARNING_FLAGS) $(CFLAGS) tests/test_analysis.c $(ANALYSIS_SOURCES) $(LDFLAGS) $(LDLIBS) -o $@

$(SANITIZER_ANALYSIS_TEST_TARGET): tests/test_analysis.c $(ANALYSIS_SOURCES) $(HEADERS)
	$(CC) $(CPPFLAGS) -Iinclude $(STANDARD_FLAGS) $(WARNING_FLAGS) $(SANITIZER_FLAGS) tests/test_analysis.c $(ANALYSIS_SOURCES) $(LDFLAGS) $(LDLIBS) -o $@

test: $(TARGET) $(RELATION_TEST_TARGET) $(ANALYSIS_TEST_TARGET)
	./$(RELATION_TEST_TARGET)
	./$(ANALYSIS_TEST_TARGET)
	./tests/run_tests.sh ./$(TARGET)

sanitize: $(SANITIZER_TARGET) $(SANITIZER_RELATION_TEST_TARGET) $(SANITIZER_ANALYSIS_TEST_TARGET)
	ASAN_OPTIONS=detect_leaks=$${ASAN_DETECT_LEAKS:-0} ./$(SANITIZER_RELATION_TEST_TARGET)
	ASAN_OPTIONS=detect_leaks=$${ASAN_DETECT_LEAKS:-0} ./$(SANITIZER_ANALYSIS_TEST_TARGET)
	ASAN_OPTIONS=detect_leaks=$${ASAN_DETECT_LEAKS:-0} ./tests/run_tests.sh ./$(SANITIZER_TARGET)

clean:
	rm -f $(TARGET) $(SANITIZER_TARGET) $(RELATION_TEST_TARGET) \
		$(SANITIZER_RELATION_TEST_TARGET) $(ANALYSIS_TEST_TARGET) \
		$(SANITIZER_ANALYSIS_TEST_TARGET)
