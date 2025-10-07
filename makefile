# Compiler and flags
CC = gcc
CFLAGS_GUI = `pkg-config --cflags gtk+-3.0` -Wall -g -Ibackend
CFLAGS_BACKEND = -Wall -g -O2 -pthread
CFLAGS_MACRO = -Wall -g
LIBS_GUI = `pkg-config --libs gtk+-3.0` -pthread -lrt
LIBS_BACKEND = -pthread -lrt
LIBS_WINDOWS = `x86_64-w64-mingw32-pkg-config --cflags gtk+-3.0` #cflags or libs?

# Target executables
TARGET_APP = autosim2
TARGET_APP_WINDOWS = windows_autosim2
TARGET_BACKEND = gpio_backend_test
TARGET_MACRO = macro_runner

# TARGET_EOL = eol_tester

# Source files
GUI_SOURCES = autosim_app/main.c autosim_app/workspace_app.c autosim_app/welcome_sceen.c autosim_app/workspace_view.c autosim_app/controls.c autosim_app/file_operations.c autosim_app/macro.c autosim_app/gpio_mapping_dialog.c

BACKEND_SOURCES = backend/gpio_backend.c backend/frontend_bridge.c
BACKEND_TEST_SOURCES = backend/backend_test.c backend/gpio_backend.c
MACRO_SOURCES = ind_macro_app/macro_runner.c

# Default target - build all applications
all: $(TARGET_APP) $(TARGET_BACKEND) $(TARGET_MACRO)

# GUI Application (with backend integration)
$(TARGET_APP): $(GUI_SOURCES) $(BACKEND_SOURCES)
	$(CC) $(CFLAGS_GUI) -o $(TARGET_APP) $(GUI_SOURCES) $(BACKEND_SOURCES) $(LIBS_GUI)

# Backend test application
$(TARGET_BACKEND): $(BACKEND_TEST_SOURCES)
	$(CC) $(CFLAGS_BACKEND) -o $(TARGET_BACKEND) $(BACKEND_TEST_SOURCES) $(LIBS_BACKEND)

# Headless macro runner
$(TARGET_MACRO): $(MACRO_SOURCES)
	$(CC) $(CFLAGS_MACRO) -o $(TARGET_MACRO) $(MACRO_SOURCES)

# Windows Target executable
$(TARGET_APP_WINDOWS): $(GUI_SOURCES)
	x86_64-w64-mingw32-gcc `pkg-config --libs gtk+-3.0` -o autosim2.exe $(GUI_SOURCES) $(LIBS_WINDOWS)

# Clean
clean:
	rm -f $(TARGET_APP) $(TARGET_BACKEND) $(TARGET_MACRO)

# Install dependencies
install-deps:
	sudo apt install libgtk-3-dev pkg-config build-essential mingw-w64 gcc-mingw-w64 mingw-w64-tools

# Test targets
gui: $(TARGET_APP)
	./$(TARGET_APP)

test-backend: $(TARGET_BACKEND)
	sudo ./$(TARGET_BACKEND)

test-macro: $(TARGET_MACRO)
	./$(TARGET_MACRO) sample_headless_macro.csv

# Ensure this is up to date and all potential make options are included 
.PHONY: all clean install-deps gui test-backend test-macro windows_autosim2