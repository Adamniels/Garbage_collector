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

demo_linked_list: demos/linked_list_demo.c src/heap.c src/allocation.c src/lib/linked_list.c src/compacting.c src/find_roots.c
	gcc -g demos/linked_list_demo.c src/heap.c src/allocation.c src/lib/linked_list.c src/compacting.c src/find_roots.c -o demo_linked_list
	./demo_linked_list
	rm -f demo_linked_list

demo_from_test: demos/demo_from_test.c src/heap.c src/allocation.c src/lib/linked_list.c src/compacting.c src/find_roots.c
	gcc -O0 -g demos/demo_from_test.c src/heap.c src/allocation.c src/lib/linked_list.c src/compacting.c src/find_roots.c -o demo_from_test
	#./demo_from_test

# Compile and run test suites with valgrind
memtest: compile_tests
	$(MEMTEST_TOOL) ./unit_tests $(MEMTEST_OPTIONS)

obj/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(C_DEBUG_FLAGS) -c $< -o $@

clean:
	rm -rf obj/*
	rm -f ./unit_tests
	rm ./demo_from_test
