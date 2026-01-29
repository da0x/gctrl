# GCtrl Makefile
# Wraps CMake build system for convenience

BUILD_DIR := build
BUILD_TYPE := Release
PREFIX := /usr/local
BINARY := gctrl

.PHONY: all build install run uninstall clean

all: build

build:
	cmake -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=$(BUILD_TYPE)
	cmake --build $(BUILD_DIR)

install: build
	install -d $(DESTDIR)$(PREFIX)/bin
	install -m 755 $(BUILD_DIR)/engine/$(BINARY) $(DESTDIR)$(PREFIX)/bin/$(BINARY)

run: build
	./$(BUILD_DIR)/engine/$(BINARY)

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/$(BINARY)

clean:
	rm -rf $(BUILD_DIR)
