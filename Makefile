RM = rm -rf
MAKEFLAGS = --no-print-directory

BUILD_DIR = $(CURDIR)/build
COMPILER = $(BUILD_DIR)/compiler
FMJCC = $(BUILD_DIR)/fmjcc
FMJINTERP = $(BUILD_DIR)/fmjinterp
ARM_CC ?= arm-linux-gnueabihf-gcc
QEMU_ARM ?= qemu-arm
LIBSYSY_ARM ?= $(CURDIR)/vendor/libsysy/libsysy_arm.a
SYSY_TEST_ROOT ?= $(CURDIR)/test
SYSY_PERF_ARCHIVE ?= /tmp/compiler2025/ARM-性能.zip
SYSY_OPT ?= -O0
OUT_DIR ?= $(CURDIR)/output
RUN_WORK ?= /tmp/sysy_run_one

.PHONY: build clean rebuild compile run run-one sysy-functional-regression sysy-performance-regression \
	legacy-build legacy-compile legacy-run legacy-run-one legacy-all-mode-regression

RUN_ONE_ARG := $(word 2,$(MAKECMDGOALS))

ifneq (,$(filter $(firstword $(MAKECMDGOALS)),run-one legacy-run-one))
ifneq ($(RUN_ONE_ARG),)
.PHONY: $(RUN_ONE_ARG)
$(RUN_ONE_ARG):
	@:
endif
endif

build:
	@cmake -G Ninja -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=Release
	@cmake --build $(BUILD_DIR)

legacy-build:
	@cmake -G Ninja -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=Release -DBUILD_LEGACY_FMJ=ON
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

sysy-functional-regression: build $(LIBSYSY_ARM)
	@COMPILER="$(COMPILER)" ARM_CC="$(ARM_CC)" QEMU_ARM="$(QEMU_ARM)" LIBSYSY_ARM="$(LIBSYSY_ARM)" \
		SYSY_TEST_ROOT="$(SYSY_TEST_ROOT)" SYSY_OPT="$(SYSY_OPT)" bash scripts/sysy_functional_regression.sh

sysy-performance-regression: build $(LIBSYSY_ARM)
	@COMPILER="$(COMPILER)" ARM_CC="$(ARM_CC)" QEMU_ARM="$(QEMU_ARM)" LIBSYSY_ARM="$(LIBSYSY_ARM)" \
		SYSY_PERF_ARCHIVE="$(SYSY_PERF_ARCHIVE)" SYSY_OPT="$(SYSY_OPT)" bash scripts/sysy_performance_regression.sh

# Legacy FMJ targets retained only for migration debugging. They are no longer
# the default workflow because test/ now contains SysY2022 programs.
LIBSYSY32 ?= $(CURDIR)/vendor/libsysy/libsysy32.s
K ?= 9
RUNTIME_CHECKS ?= 0
OPT_MODE ?= allopt
TEST_DIR ?= $(CURDIR)/legacy-test
RUN_TIMEOUT ?= 120

legacy-compile: legacy-build
	@TEST_DIR="$(TEST_DIR)" OUT_DIR="$(OUT_DIR)" FMJCC="$(FMJCC)" K="$(K)" RUNTIME_CHECKS="$(RUNTIME_CHECKS)" MODE=all \
		bash scripts/compile_submit.sh

legacy-run: legacy-build $(LIBSYSY32)
	@TEST_DIR="$(TEST_DIR)" OUT_DIR="$(OUT_DIR)" FMJCC="$(FMJCC)" ARM_CC="$(ARM_CC)" QEMU_ARM="$(QEMU_ARM)" \
		LIBSYSY32="$(LIBSYSY32)" K="$(K)" RUNTIME_CHECKS="$(RUNTIME_CHECKS)" MODE=none bash scripts/compile_submit.sh
	@OUT_DIR="$(OUT_DIR)" ARM_CC="$(ARM_CC)" QEMU_ARM="$(QEMU_ARM)" LIBSYSY32="$(LIBSYSY32)" \
		RUN_TIMEOUT="$(RUN_TIMEOUT)" bash scripts/run_submit.sh none

$(LIBSYSY32):
	@echo "Making libsysy32.s..."
	@cd "$(dir $(LIBSYSY32))" && $(ARM_CC) -mcpu=cortex-a72 -S libsysy32.c -o libsysy32.s

legacy-run-one: legacy-build $(LIBSYSY32)
	@K="$(K)" FMJCC="$(FMJCC)" ARM_CC="$(ARM_CC)" QEMU_ARM="$(QEMU_ARM)" \
		LIBSYSY32="$(LIBSYSY32)" RUN_WORK="/tmp/fmj_run_one" RUNTIME_CHECKS="$(RUNTIME_CHECKS)" OPT_MODE="$(OPT_MODE)" \
		bash scripts/run_one.sh "$(RUN_ONE_ARG)"

legacy-all-mode-regression: legacy-build $(LIBSYSY32)
	@FMJCC="$(FMJCC)" FMJINTERP="$(FMJINTERP)" ARM_CC="$(ARM_CC)" QEMU_ARM="$(QEMU_ARM)" \
		LIBSYSY32="$(LIBSYSY32)" K="$(K)" RUNTIME_CHECKS="$(RUNTIME_CHECKS)" RUN_TIMEOUT="$(RUN_TIMEOUT)" \
		bash scripts/all_mode_regression.sh
