SRC = $(wildcard gfx/*.cpp) $(wildcard *.cpp)

OUT_DIR = build
OUT = $(OUT_DIR)/aug_demo
CC = g++
FLAGS = -ggdb -Wall
INC = -Iaug -Igfx
LIBS =
LINK = -rdynamic -Wl,-rpath,.

all : $(SRC)
	$(CC) $(FLAGS) $(INC) $(SRC) $(LINK) $(LIBS) -o $(OUT)

clean: 
	rm -r $(OUT_DIR)

$(OUT_DIR):
	mkdir $(OUT_DIR)

run:
	./$(OUT)

run_mem:
	valgrind  --main-stacksize=1048576 --tool=memcheck --leak-check=full --show-leak-kinds=all \
	./$(OUT)
		