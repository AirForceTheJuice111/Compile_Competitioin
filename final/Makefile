RM = rm -rf
MAKEFLAGS = --no-print-directory

BUILD_DIR = $(CURDIR)/build
FMJCC = $(BUILD_DIR)/fmjcc

.PHONY: build clean rebuild test-one collect-tests compile-regression runtime-regression fuzz-regression

build:
	@cmake -G Ninja -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=Release
	@cmake --build $(BUILD_DIR)

clean:
	@$(RM) $(BUILD_DIR)

rebuild: clean build

FILE ?= test/insttest0
K ?= 9

test-one: build
	@$(FMJCC) --k $(K) $(FILE).fmj

collect-tests:
	@bash scripts/collect_tests.sh

compile-regression: build
	@bash scripts/compile_regression.sh

runtime-regression: build
	@bash scripts/runtime_regression.sh

fuzz-regression: build
	@bash scripts/fuzz_regression.sh
