RM = rm -rf
MAKEFLAGS = --no-print-directory

BUILD_DIR = $(CURDIR)/build
FMJCC = $(BUILD_DIR)/fmjcc
FMJINTERP = $(BUILD_DIR)/fmjinterp
ARM_CC ?= arm-linux-gnueabihf-gcc
QEMU_ARM ?= qemu-arm
LIBSYSY32 ?= $(CURDIR)/../HW12/vendor/libsysy/libsysy32.s
RUN_WORK ?= /tmp/final_run_one
TEST_DIR ?= $(CURDIR)/test
OUT_DIR ?= $(CURDIR)/output
RUN_TIMEOUT ?= 2

.PHONY: build clean rebuild compile run run-const run-loop1 run-loop2 run-allloop run-allopt test-one run-one run-one-all-mode collect-tests compile-regression runtime-regression interpreter-regression fuzz-regression all-mode-regression

RUN_ONE_ARG := $(word 2,$(MAKECMDGOALS))

ifneq (,$(filter $(firstword $(MAKECMDGOALS)),run-one run-one-all-mode))
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
	@$(RM) $(BUILD_DIR)

rebuild: clean build

FILE ?= test/insttest0
K ?= 9
RUNTIME_CHECKS ?= 0
OPT_MODE ?= allopt

compile: build
	@TEST_DIR="$(TEST_DIR)" OUT_DIR="$(OUT_DIR)" FMJCC="$(FMJCC)" K="$(K)" RUNTIME_CHECKS="$(RUNTIME_CHECKS)" MODE=all \
		bash scripts/compile_submit.sh

run: build $(LIBSYSY32)
	@TEST_DIR="$(TEST_DIR)" OUT_DIR="$(OUT_DIR)" FMJCC="$(FMJCC)" ARM_CC="$(ARM_CC)" QEMU_ARM="$(QEMU_ARM)" \
		LIBSYSY32="$(LIBSYSY32)" K="$(K)" RUNTIME_CHECKS="$(RUNTIME_CHECKS)" MODE=none \
		bash scripts/compile_submit.sh
	@OUT_DIR="$(OUT_DIR)" ARM_CC="$(ARM_CC)" QEMU_ARM="$(QEMU_ARM)" LIBSYSY32="$(LIBSYSY32)" \
		RUN_TIMEOUT="$(RUN_TIMEOUT)" bash scripts/run_submit.sh none

run-const: build $(LIBSYSY32)
	@TEST_DIR="$(TEST_DIR)" OUT_DIR="$(OUT_DIR)" FMJCC="$(FMJCC)" ARM_CC="$(ARM_CC)" QEMU_ARM="$(QEMU_ARM)" \
		LIBSYSY32="$(LIBSYSY32)" K="$(K)" RUNTIME_CHECKS="$(RUNTIME_CHECKS)" MODE=const \
		bash scripts/compile_submit.sh
	@OUT_DIR="$(OUT_DIR)" ARM_CC="$(ARM_CC)" QEMU_ARM="$(QEMU_ARM)" LIBSYSY32="$(LIBSYSY32)" \
		RUN_TIMEOUT="$(RUN_TIMEOUT)" bash scripts/run_submit.sh const

run-loop1: build $(LIBSYSY32)
	@TEST_DIR="$(TEST_DIR)" OUT_DIR="$(OUT_DIR)" FMJCC="$(FMJCC)" ARM_CC="$(ARM_CC)" QEMU_ARM="$(QEMU_ARM)" \
		LIBSYSY32="$(LIBSYSY32)" K="$(K)" RUNTIME_CHECKS="$(RUNTIME_CHECKS)" MODE=loop1 \
		bash scripts/compile_submit.sh
	@OUT_DIR="$(OUT_DIR)" ARM_CC="$(ARM_CC)" QEMU_ARM="$(QEMU_ARM)" LIBSYSY32="$(LIBSYSY32)" \
		RUN_TIMEOUT="$(RUN_TIMEOUT)" bash scripts/run_submit.sh loop1

run-loop2: build $(LIBSYSY32)
	@TEST_DIR="$(TEST_DIR)" OUT_DIR="$(OUT_DIR)" FMJCC="$(FMJCC)" ARM_CC="$(ARM_CC)" QEMU_ARM="$(QEMU_ARM)" \
		LIBSYSY32="$(LIBSYSY32)" K="$(K)" RUNTIME_CHECKS="$(RUNTIME_CHECKS)" MODE=loop2 \
		bash scripts/compile_submit.sh
	@OUT_DIR="$(OUT_DIR)" ARM_CC="$(ARM_CC)" QEMU_ARM="$(QEMU_ARM)" LIBSYSY32="$(LIBSYSY32)" \
		RUN_TIMEOUT="$(RUN_TIMEOUT)" bash scripts/run_submit.sh loop2

run-allloop: build $(LIBSYSY32)
	@TEST_DIR="$(TEST_DIR)" OUT_DIR="$(OUT_DIR)" FMJCC="$(FMJCC)" ARM_CC="$(ARM_CC)" QEMU_ARM="$(QEMU_ARM)" \
		LIBSYSY32="$(LIBSYSY32)" K="$(K)" RUNTIME_CHECKS="$(RUNTIME_CHECKS)" MODE=allloop \
		bash scripts/compile_submit.sh
	@OUT_DIR="$(OUT_DIR)" ARM_CC="$(ARM_CC)" QEMU_ARM="$(QEMU_ARM)" LIBSYSY32="$(LIBSYSY32)" \
		RUN_TIMEOUT="$(RUN_TIMEOUT)" bash scripts/run_submit.sh allloop

run-allopt: build $(LIBSYSY32)
	@TEST_DIR="$(TEST_DIR)" OUT_DIR="$(OUT_DIR)" FMJCC="$(FMJCC)" ARM_CC="$(ARM_CC)" QEMU_ARM="$(QEMU_ARM)" \
		LIBSYSY32="$(LIBSYSY32)" K="$(K)" RUNTIME_CHECKS="$(RUNTIME_CHECKS)" MODE=allopt \
		bash scripts/compile_submit.sh
	@OUT_DIR="$(OUT_DIR)" ARM_CC="$(ARM_CC)" QEMU_ARM="$(QEMU_ARM)" LIBSYSY32="$(LIBSYSY32)" \
		RUN_TIMEOUT="$(RUN_TIMEOUT)" bash scripts/run_submit.sh allopt

test-one: build
	@$(FMJCC) --k $(K) $(FILE).fmj

$(LIBSYSY32):
	@echo "Making libsysy32.s..."
	@cd "$(dir $(LIBSYSY32))" && $(ARM_CC) -mcpu=cortex-a72 -S libsysy32.c -o libsysy32.s

run-one: build $(LIBSYSY32)
	@K="$(K)" FMJCC="$(FMJCC)" ARM_CC="$(ARM_CC)" QEMU_ARM="$(QEMU_ARM)" \
		LIBSYSY32="$(LIBSYSY32)" RUN_WORK="$(RUN_WORK)" RUNTIME_CHECKS="$(RUNTIME_CHECKS)" OPT_MODE="$(OPT_MODE)" \
		bash scripts/run_one.sh "$(RUN_ONE_ARG)"

run-one-all-mode: build $(LIBSYSY32)
	@K="$(K)" FMJCC="$(FMJCC)" ARM_CC="$(ARM_CC)" QEMU_ARM="$(QEMU_ARM)" \
		LIBSYSY32="$(LIBSYSY32)" RUN_WORK="$(RUN_WORK)" RUNTIME_CHECKS="$(RUNTIME_CHECKS)" RUN_ALL_MODES=1 \
		bash scripts/run_one.sh "$(RUN_ONE_ARG)"

collect-tests:
	@bash scripts/collect_tests.sh

compile-regression: build
	@bash scripts/compile_regression.sh

runtime-regression: build
	@RUN_TIMEOUT="$(RUN_TIMEOUT)" bash scripts/runtime_regression.sh

interpreter-regression: build
	@RUN_TIMEOUT="$(RUN_TIMEOUT)" bash scripts/interpreter_regression.sh

fuzz-regression: build
	@bash scripts/fuzz_regression.sh

all-mode-regression: build $(LIBSYSY32)
	@FMJCC="$(FMJCC)" FMJINTERP="$(FMJINTERP)" ARM_CC="$(ARM_CC)" QEMU_ARM="$(QEMU_ARM)" \
		LIBSYSY32="$(LIBSYSY32)" K="$(K)" RUNTIME_CHECKS="$(RUNTIME_CHECKS)" RUN_TIMEOUT="$(RUN_TIMEOUT)" \
		bash scripts/all_mode_regression.sh
