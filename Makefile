# Memory Allocator Makefile

# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2 -g
DEBUG_FLAGS = -DDEBUG -O0 -g3
RELEASE_FLAGS = -DNDEBUG -O3 -march=native
TEST_FLAGS = -DTESTING

# Directories
SRC_DIR = src
INCLUDE_DIR = include
TEST_DIR = tests
EXAMPLES_DIR = examples
BENCHMARK_DIR = benchmarks
BUILD_DIR = build
BIN_DIR = bin

# Source files
SOURCES = $(wildcard $(SRC_DIR)/*.c)
OBJECTS = $(SOURCES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
MAIN_OBJECT = $(BUILD_DIR)/allocator.o

# Test files
TEST_SOURCES = $(wildcard $(TEST_DIR)/*.c)
TEST_BINARIES = $(TEST_SOURCES:$(TEST_DIR)/%.c=$(BIN_DIR)/%)

# Example files
EXAMPLE_SOURCES = $(wildcard $(EXAMPLES_DIR)/*.c)
EXAMPLE_BINARIES = $(EXAMPLE_SOURCES:$(EXAMPLES_DIR)/%.c=$(BIN_DIR)/%)

# Benchmark files
BENCHMARK_SOURCES = $(wildcard $(BENCHMARK_DIR)/*.c)
BENCHMARK_BINARIES = $(BENCHMARK_SOURCES:$(BENCHMARK_DIR)/%.c=$(BIN_DIR)/%)

# Main targets
.PHONY: all clean test benchmark examples install uninstall help debug release

all: directories $(BIN_DIR)/allocator_demo

# Create necessary directories
directories:
	@mkdir -p $(BUILD_DIR) $(BIN_DIR)

# Main demo executable
$(BIN_DIR)/allocator_demo: $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^
	@echo "Built main demo: $@"

# Object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -I$(INCLUDE_DIR) -c $< -o $@

# Debug build
debug: CFLAGS += $(DEBUG_FLAGS)
debug: clean all
	@echo "Debug build complete"

# Release build
release: CFLAGS += $(RELEASE_FLAGS)
release: clean all
	@echo "Release build complete"

# Test targets
test: test-build test-run

test-build: directories $(TEST_BINARIES)
	@echo "Tests built successfully"

$(BIN_DIR)/test_%: $(TEST_DIR)/test_%.c $(filter-out $(MAIN_OBJECT), $(OBJECTS))
	$(CC) $(CFLAGS) $(TEST_FLAGS) -I$(INCLUDE_DIR) -o $@ $^
	@echo "Built test: $@"

test-run: test-build
	@echo "Running all tests..."
	@for test in $(TEST_BINARIES); do \
		echo "Running $$test..."; \
		./$$test || exit 1; \
	done
	@echo "All tests passed!"

# Individual test targets
test-basic: $(BIN_DIR)/test_basic
	./$(BIN_DIR)/test_basic

test-performance: $(BIN_DIR)/test_performance
	./$(BIN_DIR)/test_performance

test-fragmentation: $(BIN_DIR)/test_fragmentation
	./$(BIN_DIR)/test_fragmentation

test-stress: $(BIN_DIR)/test_stress
	./$(BIN_DIR)/test_stress

# Example targets
examples: directories $(EXAMPLE_BINARIES)

$(BIN_DIR)/%: $(EXAMPLES_DIR)/%.c $(filter-out $(MAIN_OBJECT), $(OBJECTS))
	$(CC) $(CFLAGS) -I$(INCLUDE_DIR) -o $@ $^
	@echo "Built example: $@"

# Benchmark targets
benchmark: benchmark-build benchmark-run

benchmark-build: directories $(BENCHMARK_BINARIES)

$(BIN_DIR)/benchmark_%: $(BENCHMARK_DIR)/benchmark_%.c $(filter-out $(MAIN_OBJECT), $(OBJECTS))
	$(CC) $(CFLAGS) -I$(INCLUDE_DIR) -o $@ $^
	@echo "Built benchmark: $@"

benchmark-run: benchmark-build
	@echo "Running benchmarks..."
	@for bench in $(BENCHMARK_BINARIES); do \
		echo "Running $$bench..."; \
		./$$bench; \
	done

# Memory analysis
valgrind: $(BIN_DIR)/allocator_demo
	valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all ./$(BIN_DIR)/allocator_demo

# Performance profiling
profile: CFLAGS += -pg
profile: clean $(BIN_DIR)/allocator_demo
	./$(BIN_DIR)/allocator_demo
	gprof $(BIN_DIR)/allocator_demo gmon.out > profile_report.txt
	@echo "Profile report generated: profile_report.txt"

# Code coverage
coverage: CFLAGS += --coverage
coverage: clean test
	gcov $(SOURCES)
	@echo "Coverage files generated"

# Static analysis
analyze:
	cppcheck --enable=all --std=c99 $(SRC_DIR)/
	@echo "Static analysis complete"

# Documentation
docs:
	doxygen Doxyfile
	@echo "Documentation generated in docs/html/"

# Formatting
format:
	clang-format -i $(SRC_DIR)/*.c $(SRC_DIR)/*.h $(TEST_DIR)/*.c $(EXAMPLES_DIR)/*.c

# Installation
install: release
	sudo cp $(BIN_DIR)/allocator_demo /usr/local/bin/
	sudo cp $(INCLUDE_DIR)/*.h /usr/local/include/
	@echo "Installed to /usr/local/"

uninstall:
	sudo rm -f /usr/local/bin/allocator_demo
	sudo rm -f /usr/local/include/memory_allocator.h
	@echo "Uninstalled from /usr/local/"

# Packaging
package: release
	tar -czf memory-allocator-$(shell date +%Y%m%d).tar.gz \
		$(SRC_DIR)/ $(INCLUDE_DIR)/ $(TEST_DIR)/ $(EXAMPLES_DIR)/ \
		Makefile README.md LICENSE
	@echo "Package created: memory-allocator-$(shell date +%Y%m%d).tar.gz"

# Docker
docker-build:
	docker build -t memory-allocator .

docker-test:
	docker run --rm memory-allocator make test

# Continuous Integration
ci: clean analyze test benchmark
	@echo "CI pipeline complete"

# Performance comparison
perf-compare: $(BIN_DIR)/benchmark_compare_malloc
	./$(BIN_DIR)/benchmark_compare_malloc > performance_report.txt
	@echo "Performance comparison saved to performance_report.txt"

# Clean targets
clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)
	rm -f *.gcov *.gcda *.gcno gmon.out profile_report.txt performance_report.txt
	@echo "Cleaned build artifacts"

distclean: clean
	rm -rf docs/html/ *.tar.gz
	@echo "Cleaned all generated files"

# Help
help:
	@echo "Memory Allocator Build System"
	@echo ""
	@echo "Main targets:"
	@echo "  all              Build main demo executable"
	@echo "  debug            Build with debug flags"
	@echo "  release          Build optimized release version"
	@echo "  clean            Remove build artifacts"
	@echo "  distclean        Remove all generated files"
	@echo ""
	@echo "Testing:"
	@echo "  test             Build and run all tests"
	@echo "  test-basic       Run basic functionality tests"
	@echo "  test-performance Run performance tests"
	@echo "  test-stress      Run stress tests"
	@echo "  valgrind         Run with memory analysis"
	@echo ""
	@echo "Benchmarking:"
	@echo "  benchmark        Build and run benchmarks"
	@echo "  perf-compare     Compare with standard malloc"
	@echo "  profile          Generate performance profile"
	@echo ""
	@echo "Code Quality:"
	@echo "  analyze          Run static analysis"
	@echo "  coverage         Generate code coverage report"
	@echo "  format           Format code with clang-format"
	@echo ""
	@echo "Documentation:"
	@echo "  docs             Generate API documentation"
	@echo "  examples         Build example programs"
	@echo ""
	@echo "Deployment:"
	@echo "  install          Install to system"
	@echo "  uninstall        Remove from system"
	@echo "  package          Create distribution package"
	@echo ""
	@echo "Docker:"
	@echo "  docker-build     Build Docker image"
	@echo "  docker-test      Run tests in Docker"
	@echo ""

# Print variables (for debugging Makefile)
print-%:
	@echo $* = $($*)

# Dependency tracking
-include $(OBJECTS:.o=.d)

$(BUILD_DIR)/%.d: $(SRC_DIR)/%.c
	@$(CC) $(CFLAGS) -I$(INCLUDE_DIR) -MM -MT $(BUILD_DIR)/$*.o $< > $@