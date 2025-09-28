CC = gcc
CFLAGS = `pkg-config --cflags gtk+-3.0 json-glib-1.0`
LIBS = `pkg-config --libs gtk+-3.0 json-glib-1.0`
TARGET = workspace_app
SOURCE = main.c

$(TARGET): $(SOURCE)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCE) $(LIBS)

clean:
	rm -f $(TARGET)

install-deps:
	sudo apt install libgtk-3-dev libjson-glib-dev pkg-config

.PHONY: clean install-deps