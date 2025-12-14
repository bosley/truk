.PHONY: all clean libs apps app_one app_two

TRUK := $(CURDIR)/../../build/apps/truk/truk
BUILD_DIR := build
PKG_DIR := pkg
CMDS_DIR := cmds

LIB_A_SRC := $(PKG_DIR)/lib_a/library_a.truk
LIB_B_SRC := $(PKG_DIR)/lib_b/library_b.truk

LIB_A_H := $(BUILD_DIR)/lib_a/library_a.h
LIB_A_C := $(BUILD_DIR)/lib_a/library_a.c

LIB_B_H := $(BUILD_DIR)/lib_b/library_b.h
LIB_B_C := $(BUILD_DIR)/lib_b/library_b.c

APP_ONE_SRC := $(CMDS_DIR)/app_one/main.truk
APP_TWO_SRC := $(CMDS_DIR)/app_two/main.truk

APP_ONE_BIN := $(BUILD_DIR)/app_one
APP_TWO_BIN := $(BUILD_DIR)/app_two

all: libs apps

libs: $(LIB_A_C) $(LIB_B_C)

apps: app_one app_two

$(BUILD_DIR)/lib_a:
	mkdir -p $(BUILD_DIR)/lib_a

$(BUILD_DIR)/lib_b:
	mkdir -p $(BUILD_DIR)/lib_b

$(LIB_A_H) $(LIB_A_C): $(LIB_A_SRC) | $(BUILD_DIR)/lib_a
	@echo "Transpiling library_a to C..."
	$(TRUK) toc $(LIB_A_SRC) -o $(LIB_A_C)

$(LIB_B_H) $(LIB_B_C): $(LIB_B_SRC) | $(BUILD_DIR)/lib_b
	@echo "Transpiling library_b to C..."
	$(TRUK) toc $(LIB_B_SRC) -o $(LIB_B_C)

app_one: $(LIB_A_C) $(LIB_B_C) $(APP_ONE_SRC)
	@echo "Building app_one..."
	cd $(CMDS_DIR)/app_one && $(TRUK) main.truk -o $(CURDIR)/$(APP_ONE_BIN) -I $(CURDIR)/$(PKG_DIR)

app_two: $(LIB_B_C) $(APP_TWO_SRC)
	@echo "Building app_two..."
	cd $(CMDS_DIR)/app_two && $(TRUK) main.truk -o $(CURDIR)/$(APP_TWO_BIN) -I $(CURDIR)/$(PKG_DIR)

clean:
	rm -rf $(BUILD_DIR)

run_app_one: app_one
	@echo "\n=== Running app_one ==="
	$(APP_ONE_BIN)

run_app_two: app_two
	@echo "\n=== Running app_two ==="
	$(APP_TWO_BIN)

run_all: run_app_one run_app_two
