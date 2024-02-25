OUT_DIR = linux
CC = g++
CFLAGS = -I../aug -Wall -Werror
TARGETS = gfx

.PHONY: $(TARGETS)

all: $(TARGETS)
	echo "Done"

clean: 
	rm -r $(OUT_DIR)

$(OUT_DIR):
	mkdir $(OUT_DIR)

$(TARGETS) : $(OUT_DIR) 
	# compile to position independant object file
	$(CC) $(CFLAGS) -fpic -o $(OUT_DIR)/$@.o -c $@.cpp

	# Create a shared library from an object file
	$(CC) $(CFLAGS) -shared -o $(OUT_DIR)/$@.so $(OUT_DIR)/$@.o 
