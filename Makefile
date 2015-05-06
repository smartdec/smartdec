SRC_DIR		= $(CURDIR)/src
BUILD_DIR	= $(CURDIR)/build
MAKEFILE	= $(BUILD_DIR)/Makefile

.PHONY: all
all: tags build

.PHONY: build
build: $(MAKEFILE)
	$(MAKE) -C $(BUILD_DIR)

.PHONY: test
test: build
	$(SRC_DIR)/test-scripts/test-decompiler.py --build-dir $(BUILD_DIR)

.PHONY: test-coreutils
test-coreutils: build
	$(SRC_DIR)/test-scripts/test-decompiler.py --build-dir $(BUILD_DIR) --no-default-tests --tests-pattern "$(CURDIR)/examples/private/coreutils/*.exe"

.PHONY: test-all
test-all: build
	$(SRC_DIR)/test-scripts/test-decompiler.py --build-dir $(BUILD_DIR) --no-default-tests --tests-pattern "$(CURDIR)/examples/private/all/*.exe"

$(MAKEFILE):
	mkdir -p $(BUILD_DIR) && cd $(BUILD_DIR) && cmake $(SRC_DIR)

.PHONY: tags
tags:
	-ctags --c++-kinds=+p --fields=+iaS --extra=+q -R $(SRC_DIR)

.PHONY: doxydoc
doxydoc:
	doxygen

.PHONY: lines
lines:
	find $(SRC_DIR) \( -name "*.cpp" -o -name "*.c" -o -name "*.h" \) -print0 | xargs -0 wc

gitstats: .git
	gitstats . gitstats

.PHONY: clean
clean:
	rm -f tags gmon.out core vgcore.*
	-$(MAKE) -C $(BUILD_DIR) clean

.PHONY:
distclean: clean
	rm -rf $(BUILD_DIR) doxydoc gitstats
