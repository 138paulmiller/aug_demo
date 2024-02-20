SRC = $(wildcard *.cpp)
OUT = program
CC = g++
FLAGS = -Wall
INC = -Iaug
LIBS = -lSDL2

all : $(SRC)
	$(CC) $(INC) $(SRC) $(FLAGS) $(LIBS) -o $(OUT)

install:
	sudo apt-get install libsdl2-dev
		
run:
	./$(OUT)

run_mem:
	valgrind  --main-stacksize=1048576 --tool=memcheck --leak-check=full --show-leak-kinds=all \
	./$(OUT)
