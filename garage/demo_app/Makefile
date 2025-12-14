.PHONY: all clean libs apps run

TRUK := ../../build/apps/truk/truk
BUILD_DIR := build

all: apps

libs:
	@echo "Transpiling libraries..."
	@mkdir -p $(BUILD_DIR)/lib_a $(BUILD_DIR)/lib_b
	$(TRUK) toc pkg/lib_a/library_a.truk -o $(BUILD_DIR)/lib_a/library_a.c
	$(TRUK) toc pkg/lib_b/library_b.truk -o $(BUILD_DIR)/lib_b/library_b.c

apps: libs
	@echo "Building applications..."
	@mkdir -p $(BUILD_DIR)
	cd cmds/app_one && $(CURDIR)/$(TRUK) main.truk -o $(CURDIR)/$(BUILD_DIR)/app_one -I $(CURDIR)/pkg
	cd cmds/app_two && $(CURDIR)/$(TRUK) main.truk -o $(CURDIR)/$(BUILD_DIR)/app_two -I $(CURDIR)/pkg

run: apps
	@echo "\n=== App One ==="
	@$(BUILD_DIR)/app_one
	@echo "\n=== App Two ==="
	@$(BUILD_DIR)/app_two

clean:
	rm -rf $(BUILD_DIR)
