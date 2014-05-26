SRC_DIR		= $(CURDIR)/src
BUILD_DIR	= $(CURDIR)/build
TEST_DIR	= $(CURDIR)/tests
MAKEFILE	= $(BUILD_DIR)/Makefile

.PHONY: all
all: tags build

.PHONY: build
build: $(MAKEFILE)
	$(MAKE) -C $(BUILD_DIR)

.PHONY: test
test: build
	$(TEST_DIR)/test-decompiler.py --build-dir $(BUILD_DIR)

$(MAKEFILE):
	mkdir -p $(BUILD_DIR) && cd $(BUILD_DIR) && cmake $(SRC_DIR)

.PHONY: tags
tags:
	-ctags --c++-kinds=+p --fields=+iaS --extra=+q -R $(SRC_DIR)

.PHONY: doxydoc
doxydoc:
	doxygen

gitstats: .git
	gitstats . gitstats

.PHONY: clean
clean:
	rm -f tags gmon.out core core.* vgcore.* .ycm_extra_conf.pyc
	-$(MAKE) -C $(BUILD_DIR) clean

.PHONY:
distclean: clean
	rm -rf $(BUILD_DIR) doxydoc gitstats
