CC = gcc
CFLAGS = `pkg-config --cflags gtk+-3.0` -Wall -g
LIBS = `pkg-config --libs gtk+-3.0`
TARGET = autosim
MACRO_TARGET = macro_runner

# Headless macro runner sources
MACRO_SOURCES = ind_macro_app/macro_runner.c
MACRO_OBJECTS = ind_macro_app/macro_runner.o

all: $(TARGET) $(MACRO_TARGET)

# Full Autosim App
$(TARGET):
	$(CC) $(CFLAGS) -o $(TARGET) autosim_app/*.c $(LIBS)

# Headless Macro Runner
$(MACRO_TARGET): $(MACRO_OBJECTS)
	$(CC) $(MACRO_OBJECTS) -o $(MACRO_TARGET)

macro_runner.o: ind_macro_app/macro_runner.c ind_macro_app/macro_runner.h
	$(CC) -Wall -g -c ind_macro_app/macro_runner.c -o ind_macro_app/macro_runner.o

# Remove object files and executables
clean:
	rm -f $(TARGET) *.o $(MACRO_TARGET) $(MACRO_OBJECTS)

# Install dependencies
install-deps:
	sudo apt install libgtk-3-dev pkg-config

# Run the autosim application
run: $(TARGET)
	./$(TARGET)

.PHONY: all macro_runner clean install-deps run
