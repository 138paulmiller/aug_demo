OUT_DIR = linux
CC = gcc
CFLAGS = -I../aug -Wall -Werror
LIBS = -lSDL2 -lSDL2_ttf
TARGETS = gfx

.PHONY: $(TARGETS)

all: $(TARGETS)
	echo "Done"

clean: 
	rm -r $(OUT_DIR)

install:
	sudo apt-get install valgrind libsdl2-dev libsdl2-ttf-dev

$(OUT_DIR):
	mkdir $(OUT_DIR)

$(TARGETS) : $(OUT_DIR) 
	# compile to position independant object file
	$(CC) $(CFLAGS) -fpic -o $(OUT_DIR)/$@.o -c $@.c

	# Create a shared library from an object file
	$(CC) $(CFLAGS) -shared -o $(OUT_DIR)/$@.so $(OUT_DIR)/$@.o $(LIBS)
