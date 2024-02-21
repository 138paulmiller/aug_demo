SRC = $(wildcard aug_lib/*.cpp) $(wildcard *.cpp)
OUT = aug_demo
CC = g++
FLAGS = -Wall
INC = -Iaug -Iaug_lib
LIBS = -lSDL2 -lSDL2_ttf

all : $(SRC)
	$(CC) $(INC) $(SRC) $(FLAGS) $(LIBS) -o $(OUT)

install:
	sudo apt-get install libsdl2-dev libsdl2-ttf-dev
		
run:
	./$(OUT)

run_mem:
	#valgrind  --main-stacksize=1048576 --tool=memcheck --leak-check=full --show-leak-kinds=all \

	valgrind  --main-stacksize=1048576 --tool=memcheck \
	./$(OUT)
