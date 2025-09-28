CC = gcc
CFLAGS = `pkg-config --cflags gtk+-3.0` -Wall -g
LIBS = `pkg-config --libs gtk+-3.0`
TARGET = autosim

all: $(TARGET)

$(TARGET):
	$(CC) $(CFLAGS) -o $(TARGET) *.c $(LIBS)

clean:
	rm -f $(TARGET) *.o

install-deps:
	sudo apt install libgtk-3-dev pkg-config

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean install-deps run
