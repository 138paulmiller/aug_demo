SRC = main.cpp
OUT = aug_sdl
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
