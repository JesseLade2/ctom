CC := g++

CFLAGS := -Wall -O2
INCLUDE := -Iinclude

# Detect OS
ifeq ($(OS),Windows_NT)
    OS_NAME := windows
else
    OS_NAME := linux
endif

ifeq ($(OS_NAME),windows)
    BIN := build/ctom.exe
    RM  := del /Q
    RUN := $(BIN)
    LIBS := -lraylib -lwinmm -lgdi32 -lm -lole32 -lcomdlg32 -mwindows
else
    BIN := build/ctom
    RM  := rm -f
    RUN := ./$(BIN)
    LIBS := -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
endif

SRC := \
    src/ctom.cpp \
    src/Editor.cpp \
    src/FileManager.cpp \
    src/Terminal.cpp \
    src/Platform.cpp

all:
	$(CC) $(SRC) $(INCLUDE) $(CFLAGS) $(LIBS) -o $(BIN)

run:
	$(RUN)

clean:
	$(RM) $(BIN)
