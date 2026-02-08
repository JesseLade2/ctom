CC := g++

CFLAGS := -Wall -O2

LIBS := -lraylib -lwinmm -lgdi32 -lm -lole32 -lcomdlg32 -mwindows
BIN := build\ctom.exe
INCLUDE := -Iinclude

SRC := src\ctom.cpp src\Editor.cpp src\FileManager.cpp src\Terminal.cpp src\Platform.cpp

all:
	$(CC) $(INCLUDE) $(SRC) $(LIBS) -o $(BIN)

run:
	./ctom.exe

clean:
	del ctom.exe