RM = rm -rf
MAKEFLAGS = --no-print-directory

BUILD_DIR = $(CURDIR)/build
COMPILER = $(BUILD_DIR)/compiler
ARM_CC ?= arm-linux-gnueabihf-gcc
QEMU_ARM ?= qemu-arm
LIBSYSY_ARM ?= $(CURDIR)/vendor/libsysy/libsysy_arm.a
SYSY_TEST_ROOT ?= $(CURDIR)/test
SYSY_PERF_ARCHIVE ?= /tmp/compiler2025/ARM-性能.zip
SYSY_OPT ?=
OUT_DIR ?= $(CURDIR)/output
RUN_WORK ?= /tmp/sysy_run_one

.PHONY: build clean rebuild compile run run-one sysy-parse-regression sysy-semantic-regression sysy-functional-regression sysy-performance-regression

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

run-one: build $(LIBSYSY_ARM)
	@COMPILER="$(COMPILER)" ARM_CC="$(ARM_CC)" QEMU_ARM="$(QEMU_ARM)" LIBSYSY_ARM="$(LIBSYSY_ARM)" \
		RUN_WORK="$(RUN_WORK)" SYSY_OPT="$(SYSY_OPT)" bash scripts/sysy_run_one.sh "$(RUN_ONE_ARG)"

sysy-parse-regression: build
	@COMPILER="$(COMPILER)" SYSY_TEST_ROOT="$(SYSY_TEST_ROOT)" bash scripts/sysy_parse_regression.sh

sysy-semantic-regression: build
	@COMPILER="$(COMPILER)" SYSY_TEST_ROOT="$(SYSY_TEST_ROOT)" bash scripts/sysy_semantic_regression.sh

sysy-functional-regression: build $(LIBSYSY_ARM)
	@COMPILER="$(COMPILER)" ARM_CC="$(ARM_CC)" QEMU_ARM="$(QEMU_ARM)" LIBSYSY_ARM="$(LIBSYSY_ARM)" \
		SYSY_TEST_ROOT="$(SYSY_TEST_ROOT)" SYSY_OPT="$(SYSY_OPT)" bash scripts/sysy_functional_regression.sh

sysy-performance-regression: build $(LIBSYSY_ARM)
	@COMPILER="$(COMPILER)" ARM_CC="$(ARM_CC)" QEMU_ARM="$(QEMU_ARM)" LIBSYSY_ARM="$(LIBSYSY_ARM)" \
		SYSY_PERF_ARCHIVE="$(SYSY_PERF_ARCHIVE)" SYSY_OPT="$(SYSY_OPT)" bash scripts/sysy_performance_regression.sh
