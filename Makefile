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
SOURCES := main.c grafo.c
HEADERS := grafo.h

.PHONY: all test sanitize clean

all: $(TARGET)

$(TARGET): $(SOURCES) $(HEADERS)
	$(CC) $(CPPFLAGS) $(STANDARD_FLAGS) $(WARNING_FLAGS) $(CFLAGS) $(SOURCES) $(LDFLAGS) $(LDLIBS) -o $@

$(SANITIZER_TARGET): $(SOURCES) $(HEADERS)
	$(CC) $(CPPFLAGS) $(STANDARD_FLAGS) $(WARNING_FLAGS) $(SANITIZER_FLAGS) $(SOURCES) $(LDFLAGS) $(LDLIBS) -o $@

test: $(TARGET)
	./tests/run_tests.sh ./$(TARGET)

sanitize: $(SANITIZER_TARGET)
	ASAN_OPTIONS=detect_leaks=$${ASAN_DETECT_LEAKS:-0} ./tests/run_tests.sh ./$(SANITIZER_TARGET)

clean:
	rm -f $(TARGET) $(SANITIZER_TARGET)
