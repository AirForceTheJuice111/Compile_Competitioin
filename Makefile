RM = rm -rf
MAKEFLAGS = --no-print-directory

BUILD_DIR = $(CURDIR)/build
COMPILER = $(BUILD_DIR)/compiler
AARCH64_CC ?= clang
AARCH64_CC_FLAGS ?= --target=aarch64-linux-gnu
QEMU_AARCH64 ?= qemu-aarch64
SYSY_AARCH64_SYSROOT ?= /usr/aarch64-linux-gnu
LIBSYSY_AARCH64_C ?= $(CURDIR)/vendor/libsysy/sylib.c
SYSY_TEST_ROOT ?= $(CURDIR)/test
SYSY_PERF_ARCHIVE ?= /tmp/compiler2025/ARM-性能.zip
SYSY_OPT ?=
OUT_DIR ?= $(CURDIR)/output
RUN_WORK ?= /tmp/sysy_run_one

.PHONY: build clean rebuild compile run run-one sysy-parse-regression sysy-semantic-regression sysy-functional-regression sysy-performance-regression sysy-parallel-native-regression sysy-parallel-plan-regression

RUN_ONE_ARG := $(word 2,$(MAKECMDGOALS))

ifneq (,$(filter $(firstword $(MAKECMDGOALS)),run-one))
ifneq ($(RUN_ONE_ARG),)
.PHONY: $(RUN_ONE_ARG)
$(RUN_ONE_ARG):
	@:
endif
endif

build:
	@cmake -G Ninja -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=Release
	@cmake --build $(BUILD_DIR)

clean:
	@$(RM) $(BUILD_DIR) $(OUT_DIR)

rebuild: clean build

compile: build
	@COMPILER="$(COMPILER)" SYSY_TEST_ROOT="$(SYSY_TEST_ROOT)" OUT_DIR="$(OUT_DIR)" SYSY_OPT="$(SYSY_OPT)" \
		bash scripts/sysy_compile_all.sh

run: sysy-functional-regression

run-one: build
	@COMPILER="$(COMPILER)" AARCH64_CC="$(AARCH64_CC)" AARCH64_CC_FLAGS="$(AARCH64_CC_FLAGS)" \
		QEMU_AARCH64="$(QEMU_AARCH64)" SYSY_AARCH64_SYSROOT="$(SYSY_AARCH64_SYSROOT)" \
		LIBSYSY_AARCH64_C="$(LIBSYSY_AARCH64_C)" RUN_WORK="$(RUN_WORK)" SYSY_OPT="$(SYSY_OPT)" \
		bash scripts/sysy_run_one.sh "$(RUN_ONE_ARG)"

sysy-parse-regression: build
	@COMPILER="$(COMPILER)" SYSY_TEST_ROOT="$(SYSY_TEST_ROOT)" bash scripts/sysy_parse_regression.sh

sysy-semantic-regression: build
	@COMPILER="$(COMPILER)" SYSY_TEST_ROOT="$(SYSY_TEST_ROOT)" bash scripts/sysy_semantic_regression.sh

sysy-functional-regression: build
	@COMPILER="$(COMPILER)" AARCH64_CC="$(AARCH64_CC)" AARCH64_CC_FLAGS="$(AARCH64_CC_FLAGS)" \
		QEMU_AARCH64="$(QEMU_AARCH64)" SYSY_AARCH64_SYSROOT="$(SYSY_AARCH64_SYSROOT)" \
		LIBSYSY_AARCH64_C="$(LIBSYSY_AARCH64_C)" SYSY_TEST_ROOT="$(SYSY_TEST_ROOT)" SYSY_OPT="$(SYSY_OPT)" \
		bash scripts/sysy_functional_regression.sh

sysy-performance-regression: build
	@COMPILER="$(COMPILER)" AARCH64_CC="$(AARCH64_CC)" AARCH64_CC_FLAGS="$(AARCH64_CC_FLAGS)" \
		QEMU_AARCH64="$(QEMU_AARCH64)" SYSY_AARCH64_SYSROOT="$(SYSY_AARCH64_SYSROOT)" \
		LIBSYSY_AARCH64_C="$(LIBSYSY_AARCH64_C)" SYSY_PERF_ARCHIVE="$(SYSY_PERF_ARCHIVE)" SYSY_OPT="$(SYSY_OPT)" \
		bash scripts/sysy_performance_regression.sh

sysy-parallel-native-regression: build
	@COMPILER="$(COMPILER)" AARCH64_CC="$(AARCH64_CC)" AARCH64_CC_FLAGS="$(AARCH64_CC_FLAGS)" \
		QEMU_AARCH64="$(QEMU_AARCH64)" SYSY_AARCH64_SYSROOT="$(SYSY_AARCH64_SYSROOT)" \
		LIBSYSY_AARCH64_C="$(LIBSYSY_AARCH64_C)" bash scripts/sysy_parallel_native_regression.sh

sysy-parallel-plan-regression: build
	@COMPILER="$(COMPILER)" SYSY_TEST_ROOT="$(CURDIR)/test/functional" \
		bash scripts/sysy_parallel_plan_regression.sh
