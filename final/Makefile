RM = rm -rf
MAKEFLAGS = --no-print-directory

BUILD_DIR = $(CURDIR)/build
FMJCC = $(BUILD_DIR)/fmjcc
FMJINTERP = $(BUILD_DIR)/fmjinterp
ARM_CC ?= arm-linux-gnueabihf-gcc
QEMU_ARM ?= qemu-arm
LIBSYSY32 ?= $(CURDIR)/../HW12/vendor/libsysy/libsysy32.s
RUN_WORK ?= /tmp/final_run_one

.PHONY: build clean rebuild test-one run-one collect-tests compile-regression runtime-regression interpreter-regression fuzz-regression

RUN_ONE_ARG := $(word 2,$(MAKECMDGOALS))

ifeq ($(firstword $(MAKECMDGOALS)),run-one)
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

test-one: build
	@$(FMJCC) --k $(K) $(FILE).fmj

$(LIBSYSY32):
	@echo "Making libsysy32.s..."
	@cd "$(dir $(LIBSYSY32))" && $(ARM_CC) -mcpu=cortex-a72 -S libsysy32.c -o libsysy32.s

run-one: build $(LIBSYSY32)
	@K="$(K)" FMJCC="$(FMJCC)" ARM_CC="$(ARM_CC)" QEMU_ARM="$(QEMU_ARM)" \
		LIBSYSY32="$(LIBSYSY32)" RUN_WORK="$(RUN_WORK)" \
		bash scripts/run_one.sh "$(RUN_ONE_ARG)"

collect-tests:
	@bash scripts/collect_tests.sh

compile-regression: build
	@bash scripts/compile_regression.sh

runtime-regression: build
	@bash scripts/runtime_regression.sh

interpreter-regression: build
	@bash scripts/interpreter_regression.sh

fuzz-regression: build
	@bash scripts/fuzz_regression.sh
