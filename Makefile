CC = gcc -O0
C_DEBUG_FLAGS = -Wall -Wextra -g -pedantic
CUNIT_INCLUDE = -lcunit
PROFILER = gprof
MEMTEST_TOOL = valgrind
MEMTEST_OPTIONS = --leak-check=full

SOURCE_FILES = $(shell find src -type f -name '*.c')
SOURCE_OBJECTS = $(patsubst src/%.c,obj/src/%.o,$(SOURCE_FILES))
DEMO_FILES = $(wildcard demo/*.c)
DEMO_OBJECTS = $(patsubst demo/%.c,obj/demo/%.o,$(DEMO_FILES))
TEST_FILES = $(wildcard test/*.c)
TEST_OBJECTS = $(patsubst test/%.c,obj/test/%.o,$(TEST_FILES))

.PHONY: clean test memtest demo

compile: $(SOURCE_OBJECTS)

# Compile test suites
compile_tests: compile $(TEST_OBJECTS)
	$(CC) $(SOURCE_OBJECTS) $(TEST_OBJECTS) -o unit_tests $(CUNIT_INCLUDE) 

# Compile and run test suites
test: compile_tests
	./unit_tests

# Compile and run test suites with valgrind
memtest: compile_tests
	$(MEMTEST_TOOL) ./unit_tests $(MEMTEST_OPTIONS)

obj/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(C_DEBUG_FLAGS) -c $< -o $@

clean:
	rm -rf obj/*
	rm -f ./unit_tests
