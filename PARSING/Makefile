RM       = rm -rf
MAKEFLAGS = --no-print-directory

.PHONY: build clean rebuild handin

BUILD_DIR = $(CURDIR)/build

build:
	@cmake -G Ninja -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=Release; \
	cd $(BUILD_DIR) && ninja
# cd $(BUILD_DIR) && ninja -v

clean: 
	@$(RM) build ; \
	$(RM) test/*.ast ; \

rebuild: clean build

.PHONY: run run-one run-extra

MAIN = $(BUILD_DIR)/tools/main/main

run: $(MAIN)
	cd $(CURDIR)/test && \
	for file in $$(ls .); do \
        if [ "$${file##*.}" = "fmj" ]; then \
            echo "Parsing $${file%%.*}"; \
			$(MAIN) "$${file%%.*}"; \
        fi \
	done; \
	cd .. > /dev/null 2>&1 

run-one: $(MAIN)
	cd $(CURDIR)/test && \
	echo "Parsing ${FILE}.fmj"; \
	$(MAIN) "${FILE}"; \
	cd .. > /dev/null 2>&1 
